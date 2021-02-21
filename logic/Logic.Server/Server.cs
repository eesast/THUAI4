using System;
using System.Collections.Generic;
using System.Text;
using THUnity2D;
using Communication.Proto;
using System.Threading.Tasks;
using System.Threading;

namespace Logic.Server
{
	class Server
	{
		public const int SendMessageToClientIntervalInMilliseconds = 50;	//每隔xx毫秒向客户端发送信息

		private Map game;
		private ArgumentOptions options;
		private uint GetBirthPointIdx(long teamID, long playerID)		//获取出生点位置
		{
			return (uint)(teamID * options.PlayerCountPerTeam + playerID);
		}

		private long[,] communicationToGameID;      //把通信所用ID映射到游戏ID，[i, j]代表第 i 个队伍的第 j 个玩家的 id
		private object addPlayerLock = new object();
		private bool ValidTeamIDAndPlayerID(long teamID, long playerID)		//判断是否是合法的TeamID与PlayerID
		{
			return teamID >= 0 && teamID < options.TeamCount && playerID >= 0 && playerID < options.PlayerCountPerTeam;
		}

		private Communication.CommServer.CommServer serverCommunicator;

		public Server(ArgumentOptions options)
		{
			//队伍数量在 1~4 之间，总人数不超过 8
			if (options.TeamCount > 4) options.TeamCount = 4;
			if (options.TeamCount < 1) options.TeamCount = 1;
			if (options.PlayerCountPerTeam * options.TeamCount > 8) options.PlayerCountPerTeam = (ushort)(8 / options.TeamCount);
			if (options.PlayerCountPerTeam < 1) options.PlayerCountPerTeam = 1;

			this.options = options;
			game = new Map(MapInfo.map, options.TeamCount);
			communicationToGameID = new long[options.TeamCount, options.PlayerCountPerTeam];
			for (int i = 0; i < communicationToGameID.GetLength(0); ++i)
			{
				for (int j = 0; j < communicationToGameID.GetLength(1); ++j)
				{
					communicationToGameID[i, j] = GameObject.invalidID;		//初始值为无效ID，代表还未进入游戏
				}
			}

			/*TODO: 绑定OnReceive、开始监听，未完成*/
			serverCommunicator = new Communication.CommServer.CommServer();

			while (!serverCommunicator.Listen(options.ServerPort))
			{
				Console.WriteLine("Server listen failed!");
				Thread.Sleep(1000);
			}

			Console.WriteLine("Server begins to listen!");

			serverCommunicator.OnConnect += delegate ()
			{
				Console.WriteLine("Successfully connected!");
			};

			serverCommunicator.OnReceive += delegate ()
			{
				/*TODO: 绑定OnReceive未完成*/

				//IMsg msg;
				//if (serverCommunicator.TryTake(out IMsg msg))
			};
		}

		private void OnReceive(MessageToServer msg)
		{
			switch (msg.MessageType)
			{
			case MessageType.AddPlayer:
				SendAddPlayerResponse(msg, AddPlayer(msg));
				break;
			case MessageType.Move:
				if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
				{
					game.MovePlayer(communicationToGameID[msg.TeamID, msg.PlayerID], msg.TimeInMilliseconds, msg.Angle);
				}
				break;
			case MessageType.Attack:
				if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
				{
					game.Attack(communicationToGameID[msg.TeamID, msg.PlayerID], msg.TimeInMilliseconds, msg.Angle);
				}
				break;
			case MessageType.Pick:
				if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
				{
					game.Pick(communicationToGameID[msg.TeamID, msg.PlayerID], ConvertTool.ToGamePropType(msg.PropType));
				}
				break;
			case MessageType.Use:
				if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
				{
					game.Use(communicationToGameID[msg.TeamID, msg.PlayerID]);
				}
				break;
			case MessageType.Throw:
				if (ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID))
				{
					game.Throw(communicationToGameID[msg.TeamID, msg.PlayerID], msg.TimeInMilliseconds, msg.Angle);
				}
				break;
			}
		}

		private void SendAddPlayerResponse(MessageToServer msgRecv, bool isValid)	//作为对 AddPlayer 消息的响应，给客户端发送信息
		{
			MessageToOneClient msg2Send = new MessageToOneClient();
			msg2Send.PlayerID = msgRecv.PlayerID;
			msg2Send.TeamID = msgRecv.TeamID;
			msg2Send.MessageType = isValid ? MessageType.ValidPlayer : MessageType.InvalidPlayer;

			/*TODO: 发送消息，未完成*/
		}

		private bool AddPlayer(MessageToServer msg)
		{
			if (game.IsGaming)		//如果正在游戏，不能加入角色
			{
				return false;
			}

			if (!ValidTeamIDAndPlayerID(msg.TeamID, msg.PlayerID)) //PlayerID或TeamID不正确
			{
				return false;
			}
			
			lock (addPlayerLock)
			{

				if (communicationToGameID[msg.TeamID, msg.PlayerID] != GameObject.invalidID)        //已经有这个玩家了，不能再添加了
				{
					return false;
				}

				Map.PlayerInitInfo playerInitInfo = new Map.PlayerInitInfo(GetBirthPointIdx(msg.TeamID, msg.PlayerID), ConvertTool.ToGameJobType(msg.JobType), msg.TeamID);
				if (playerInitInfo.jobType == THUnity2D.JobType.InvalidJobType) return false;       //非法职业

				bool legalJob = false;
				foreach (var enumMem in typeof(THUnity2D.JobType).GetFields())
				{
					if (playerInitInfo.jobType.ToString() == enumMem.Name)
					{
						legalJob = true;
						break;
					}
				}
				if (!legalJob) return false;		//非法职业，职业数值超出枚举范围

				long newPlayerID = game.AddPlayer(playerInitInfo);
				if (newPlayerID == GameObject.invalidID) return false;
				communicationToGameID[msg.TeamID, msg.PlayerID] = newPlayerID;

				CheckStart();		//检查是否该开始游戏了
				return true;
			}
		}

		private void CheckStart()           //检查是否满员，该开始游戏了
		{

			if (game.IsGaming) return;

			foreach (var id in communicationToGameID)
			{
				if (id == GameObject.invalidID) return;		//如果有未初始化的玩家，不开始游戏
			}

			Task.Run
				(
					() =>
					{
						//开始游戏
						game.StartGame((int)(options.GameTimeInSecond * 1000));

						//游戏结束
						OnGameEnd();
					}
				);
			SendMessageToAllClients(MessageType.StartGame);     //发送开始游戏信息

			//开始每隔一定时间向客户端发送游戏情况
			Task.Run
				(
					() =>
					{
						while (!game.IsGaming) Thread.Sleep(1);	//游戏未开始，等待
						while (game.IsGaming)
						{
							SendMessageToAllClients(MessageType.Gaming);
							Thread.Sleep(SendMessageToClientIntervalInMilliseconds);
						}
					}
				);
		}

		private void OnGameEnd()		//游戏结束后的行为
		{
			//向所有玩家发送结束游戏消息
			SendMessageToAllClients(MessageType.EndGame);
		}
		
		private void SendMessageToAllClients(MessageType msgType)		//向所有的客户端发送消息
		{
			var gameObjList = game.GetGameObject();
			var cellColor = game.CellColor;

			var rows = game.Rows;
			var cols = game.Cols;

			//记录颜色信息，避免重复构造颜色信息
			Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneDimVec> msgCellColors = new Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneDimVec>();
			for (int x = 0; x < rows; ++x)
			{
				msgCellColors.Add(new MessageToClient.Types.OneDimVec());
				for (int y = 0; y < cols; ++y)
				{
					msgCellColors[x].RowColors.Add(ConvertTool.ToCommunicationColorType(cellColor[x, y]));
				}
			}

			//记录所有游戏对象信息，避免重复构造游戏对象信息
			Google.Protobuf.Collections.RepeatedField<GameObjInfo> msgGameObjs = new Google.Protobuf.Collections.RepeatedField<GameObjInfo>();
			foreach (var gameObj in gameObjList)
			{
				msgGameObjs.Add(CopyInfo.Auto((GameObject)gameObj));
			}

			//记录所有GUID信息
			Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneTeamGUIDs> playerGUIDs = new Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneTeamGUIDs>();
			for (int x = 0; x < options.TeamCount; ++x)
			{
				playerGUIDs.Add(new MessageToClient.Types.OneTeamGUIDs());
				for (int y = 0; y < options.PlayerCountPerTeam; ++y)
				{
					playerGUIDs[x].TeammateGUIDs.Add(communicationToGameID[x, y]);
				}
			}
				

			for (int i = 0; i < options.TeamCount; ++i)
			{
				int teamScore = game.GetTeamScore(i);
				for (int j = 0; j < options.PlayerCountPerTeam; ++j)
				{
					MessageToClient msg = new MessageToClient();
					msg.TeamID = i;
					msg.PlayerID = j;
					msg.MessageType = msgType;
					msg.SelfInfo = CopyInfo.Player(game.GetPlayerFromTeam(communicationToGameID[i, j]));

					for (int k = 0; k < options.TeamCount; ++k)
					{
						msg.PlayerGUIDs.Add(playerGUIDs[k]);
					}

					msg.SelfTeamColor = ConvertTool.ToCommunicationColorType(game.TeamToColor(i));

					foreach (var infos in msgGameObjs)
					{
						msg.GameObjs.Add(infos);
					}

					for (int x = 0; x < rows; ++x)
					{
						msg.CellColors.Add(msgCellColors[x]);
					}

					msg.TeamScore = teamScore;

					/*TODO: 向该玩家发送消息，未完成*/
				}
			}
		}

	}
}

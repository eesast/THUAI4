using GameEngine;
using System;
using System.Collections;
using System.Threading;
using THUnity2D;

namespace Gaming
{
	public partial class Game
	{
		public struct PlayerInitInfo
		{
			public uint birthPointIdx;
			public JobType jobType;
			public long teamID;
			public PlayerInitInfo(uint birthPointIdx, JobType jobType, long teamID)
			{
				this.birthPointIdx = birthPointIdx;
				this.jobType = jobType;
				this.teamID = teamID;
			}
		}
		
		public const int maxTeamNum = 4;

		private ArrayList teamList;                     // 队伍列表
														//private object teamListLock = new object();	// 队伍暂时不需要锁
		private readonly int numOfTeam;

		public long AddPlayer(PlayerInitInfo playerInitInfo)
		{
			if (!Team.teamExists(playerInitInfo.teamID)
				|| !MapInfo.ValidBirthPointIdx(playerInitInfo.birthPointIdx)
				|| gameMap.BirthPointList[playerInitInfo.birthPointIdx].Parent != null) return GameObject.invalidID;

			XYPosition pos = gameMap.BirthPointList[playerInitInfo.birthPointIdx].Position;
			Character newPlayer = new Character(pos, Map.Constant.playerRadius, playerInitInfo.jobType, Map.Constant.basicPlayerMoveSpeed);
			gameMap.BirthPointList[playerInitInfo.birthPointIdx].Parent = newPlayer;
			gameMap.PlayerListLock.EnterWriteLock(); try { gameMap.PlayerList.Add(newPlayer); } finally { gameMap.PlayerListLock.ExitWriteLock(); }
			((Team)teamList[(int)playerInitInfo.teamID]).AddPlayer(newPlayer);
			newPlayer.TeamID = playerInitInfo.teamID;

			//设置出生点的颜色

			int cellX = Map.Constant.GridToCellX(pos), cellY = Map.Constant.GridToCellY(pos);
			gameMap.SetCellColor(cellX, cellY, Map.TeamToColor(playerInitInfo.teamID));

			//开启装弹线程

			new Thread
				(
					() =>
					{
						while (!gameMap.Timer.IsGaming) Thread.Sleep(newPlayer.CD);
						while (gameMap.Timer.IsGaming)
						{
							var beginTime = Environment.TickCount64;

							var cellX = Map.Constant.GridToCellX(newPlayer.Position);
							var cellY = Map.Constant.GridToCellY(newPlayer.Position);
							if (gameMap.GetCellColor(cellX, cellY) == Map.TeamToColor(newPlayer.TeamID)) newPlayer.AddBulletNum();

							var endTime = Environment.TickCount64;
							var deltaTime = endTime - beginTime;
							if (deltaTime < newPlayer.CD)
							{
								Thread.Sleep(newPlayer.CD - (int)deltaTime);
							}
							else
							{
								Console.WriteLine("The computer runs so slow that the player cannot finish adding bullet during this time!!!!!!");
							}
						}
					}
				)
			{ IsBackground = true }.Start();
			return newPlayer.ID;
		}

		public bool StartGame(int milliSeconds)
		{
			if (gameMap.Timer.IsGaming) return false;
			gameMap.PlayerListLock.EnterReadLock();
			try
			{
				foreach (Character player in gameMap.PlayerList)
				{
					player.CanMove = true;
					player.AddShield(Map.Constant.shieldTimeAtBirth);       //出生时附加盾牌
				}
			}
			finally { gameMap.PlayerListLock.ExitReadLock(); }

			//开始产生道具

			propManager.StartProducing();

			if (!gameMap.Timer.StartGame(milliSeconds)) return false;

			gameMap.PlayerListLock.EnterWriteLock();
			try
			{
				foreach (Character player in gameMap.PlayerList)
				{
					player.CanMove = false;
				}
				gameMap.PlayerList.Clear();
			}
			finally { gameMap.PlayerListLock.ExitWriteLock(); }
			gameMap.ObjListLock.EnterWriteLock();
			try
			{
				gameMap.ObjList.Clear();
			}
			finally { gameMap.ObjListLock.ExitWriteLock(); }
			return true;
		}

		//人物移动
		public void MovePlayer(long playerID, int moveTimeInMilliseconds, double moveDirection)
		{
			if (!gameMap.Timer.IsGaming) return;
			Character? playerToMove = gameMap.FindPlayer(playerID);
			if (playerToMove != null) moveManager.MovePlayer(playerToMove, moveTimeInMilliseconds, moveDirection);
		}

		public bool Attack(long playerID, int timeInMilliseconds, double angle)
		{
			if (!gameMap.Timer.IsGaming) return false;
			Character? playerWillAttack = gameMap.FindPlayer(playerID);
			return playerWillAttack == null ? false : attackManager.Attack(playerWillAttack, timeInMilliseconds, angle);
		}

		//捡道具，是否是前面的那一格（true则是面向的那一格；false则是所在的那一格），以及要捡的道具类型
		public bool Pick(long playerID, PropType propType)
		{
			if (!gameMap.Timer.IsGaming) return false;
			Character? player = gameMap.FindPlayer(playerID);
			if (player == null) return false;

			Prop? prop = propManager.PickProp(player, propType);

			player.IsModifyingProp = false;
			return prop != null;
		}

		public void Use(long playerID)
		{
			if (!gameMap.Timer.IsGaming) return;
			Character? player = gameMap.FindPlayer(playerID);
			if (player == null) return;

			propManager.UseProp(player);
		}

		public void SendMessage(long fromID, long toID, string message)
		{
			if (message.Length > 64) return;
			Character from, to;
			try
			{
				from = GetPlayerFromTeam(fromID);
				to = GetPlayerFromTeam(toID);
			}
			catch { return; }

			if (from.TeamID != to.TeamID) return;

			to.Message = message;
		}

		public ArrayList GetGameObject()
		{
			ArrayList gameObjList = new ArrayList();
			foreach (Team team in teamList)     // team 只有在开始游戏之前被修改，开始之后是只读的，因此不须加锁
			{
				gameObjList.AddRange(team.GetPlayerListForUnsafe());
			}
			gameMap.ObjListLock.EnterWriteLock(); try { gameObjList.AddRange(gameMap.ObjList); } finally { gameMap.ObjListLock.ExitWriteLock(); }
			propManager.UnpickedPropListLock.EnterReadLock(); try { gameObjList.AddRange(propManager.UnpickedPropList); } finally { propManager.UnpickedPropListLock.ExitReadLock(); }
			return gameObjList;
		}

		public Character GetPlayerFromTeam(long playerID)   //从队伍中寻找玩家，要求一定要找到
		{
			foreach (Team team in teamList)
			{
				Character? player = team.GetPlayer(playerID);
				if (player != null) return player;
			}
			throw new Exception("GetPlayerFromTeam error: No this player!");
		}

		public long[] GetPlayerIDsOfTheTeam(long teamID)
		{
			return ((Team)teamList[(int)teamID]).GetPlayerIDs();
		}

		public void Throw(long playerID, int moveTimeInMilliseconds, double angle)
		{
			Character? player = gameMap.FindPlayer(playerID);
			if (player == null) return;
			propManager.ThrowProp(player, moveTimeInMilliseconds, angle);
		}

		public int GetTeamScore(long teamID)
		{
			if (!Team.teamExists(teamID)) throw new Exception("");
			return gameMap.GetColorArea(Map.TeamToColor(teamID));
		}

		private Map gameMap;
		public Map GameMap => gameMap;
		public Game(uint[,] mapResource, int numOfTeam)
		{
			if (numOfTeam > maxTeamNum) throw new TeamNumOverFlowException();

			gameMap = new Map(mapResource);

			//加入队伍
			this.numOfTeam = numOfTeam;
			teamList = new ArrayList();
			for (int i = 0; i < numOfTeam; ++i)
			{
				teamList.Add(new Team());
			}

			propManager = new PropManager(gameMap);
			attackManager = new AttackManager(gameMap);
			moveManager = new MoveManager(gameMap);
		}
	}

	public class TeamNumOverFlowException : Exception
	{
		public override string Message => "Number of teams overflows!";
	}

	public class TeamNotExistException : Exception
	{
		public override string Message => "The team sprcified by the teamID not exists!";
	}
}

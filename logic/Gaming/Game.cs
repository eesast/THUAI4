using GameEngine;
using System;
using System.Collections.Generic;
using System.Threading;
using THUnity2D;
using Timothy.FrameRateTask;

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
		private const long checkColorInterval = 50;		// 检查脚下颜色间隔时间
		private List<Team> teamList;                     // 队伍列表
		//private object teamListLock = new object();	// 队伍暂时不需要锁
		private readonly int numOfTeam;

		public long AddPlayer(PlayerInitInfo playerInitInfo)
		{
			if (!Team.teamExists(playerInitInfo.teamID)
				|| !MapInfo.ValidBirthPointIdx(playerInitInfo.birthPointIdx)
				|| gameMap.BirthPointList[playerInitInfo.birthPointIdx].Parent != null) return GameObject.invalidID;

			XYPosition pos = gameMap.BirthPointList[playerInitInfo.birthPointIdx].Position;
			Character? newPlayer = Character.GetCharacter(pos, Constant.playerRadius, Constant.basicPlayerMoveSpeed, playerInitInfo.jobType);
			if (newPlayer == null) return GameObject.invalidID;

			gameMap.BirthPointList[playerInitInfo.birthPointIdx].Parent = newPlayer;
			gameMap.PlayerListLock.EnterWriteLock(); try { gameMap.PlayerList.Add(newPlayer); } finally { gameMap.PlayerListLock.ExitWriteLock(); }
			teamList[(int)playerInitInfo.teamID].AddPlayer(newPlayer);
			newPlayer.TeamID = playerInitInfo.teamID;

			//设置出生点的颜色

			int cellX = Constant.GridToCellX(pos), cellY = Constant.GridToCellY(pos);
			gameMap.SetCellColor(cellX, cellY, Map.TeamToColor(playerInitInfo.teamID));

			//开启装弹线程

			new Thread
				(
					() =>
					{
						while (!gameMap.Timer.IsGaming) Thread.Sleep(newPlayer.CD);

						long addBulletTime = long.MaxValue;

						new FrameRateTaskExecutor<int>
						(
							loopCondition: () => gameMap.Timer.IsGaming,
							loopToDo: () =>
							{
								var cellX = Constant.GridToCellX(newPlayer.Position);
								var cellY = Constant.GridToCellY(newPlayer.Position);
								if (gameMap.GetCellColor(cellX, cellY) == Map.TeamToColor(newPlayer.TeamID))
								{
									var nowTime = Environment.TickCount64;
									if (nowTime >= addBulletTime)
									{
										newPlayer.AddBulletNum();
										addBulletTime = nowTime + newPlayer.CD;
									}
									else if (nowTime + newPlayer.CD < addBulletTime)
									{
										addBulletTime = nowTime + newPlayer.CD;
									}
								}
								else
								{
									addBulletTime = long.MaxValue;
								}
							},
							timeInterval: checkColorInterval,
							() => 0
						)
						{
							AllowTimeExceed = true,
							MaxTolerantTimeExceedCount = 5,
							TimeExceedAction = exceedTooMuch =>
							{
								if (exceedTooMuch) Console.WriteLine("The computer runs too slow that it cannot check the color below the player in time!");
							}
						}.Start();
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
					player.AddShield(Constant.shieldTimeAtBirth);       //出生时附加盾牌
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

		/// <summary>
		/// 人物移动
		/// </summary>
		/// <param name="playerID">人物的ID</param>
		/// <param name="moveTimeInMilliseconds">移动时间，毫秒</param>
		/// <param name="moveDirection">移动方向，弧度</param>
		public void MovePlayer(long playerID, int moveTimeInMilliseconds, double moveDirection)
		{
			if (!gameMap.Timer.IsGaming) return;
			Character? playerToMove = gameMap.FindPlayer(playerID);
			if (playerToMove != null) moveManager.MovePlayer(playerToMove, moveTimeInMilliseconds, moveDirection);
		}

		/// <summary>
		/// 攻击
		/// </summary>
		/// <param name="playerID">人物ID</param>
		/// <param name="timeInMilliseconds">子弹飞行时间，毫秒</param>
		/// <param name="angle">子弹飞行角度，弧度</param>
		/// <returns>是否攻击成功</returns>
		public bool Attack(long playerID, int timeInMilliseconds, double angle)
		{
			if (!gameMap.Timer.IsGaming) return false;
			Character? playerWillAttack = gameMap.FindPlayer(playerID);
			return playerWillAttack == null ? false : attackManager.Attack(playerWillAttack, timeInMilliseconds, angle);
		}

		/// <summary>
		/// 捡道具
		/// </summary>
		/// <param name="playerID">捡道具的人物ID</param>
		/// <param name="propType">要捡的道具类型</param>
		/// <returns>是否捡道具成功</returns>
		public bool Pick(long playerID, PropType propType)
		{
			if (!gameMap.Timer.IsGaming) return false;
			Character? player = gameMap.FindPlayer(playerID);
			if (player == null) return false;

			Prop? prop = propManager.PickProp(player, propType);

			player.IsModifyingProp = false;
			return prop != null;
		}

		/// <summary>
		/// 使用道具
		/// </summary>
		/// <param name="playerID">捡道具的人物ID</param>
		public void Use(long playerID)
		{
			if (!gameMap.Timer.IsGaming) return;
			Character? player = gameMap.FindPlayer(playerID);
			if (player == null) return;

			propManager.UseProp(player);
		}

		/// <summary>
		/// 发送消息
		/// </summary>
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

		/// <summary>
		/// 获取当前场上的对象，和已经下场的玩家
		/// </summary>
		public List<IGameObj> GetGameObject()
		{
			var gameObjList = new List<IGameObj>();
			foreach (Team team in teamList)     // team 只有在开始游戏之前被修改，开始之后是只读的，因此不须加锁
			{
				gameObjList.AddRange(team.GetPlayerListForUnsafe());
			}
			gameMap.ObjListLock.EnterWriteLock(); try { gameObjList.AddRange(gameMap.ObjList); } finally { gameMap.ObjListLock.ExitWriteLock(); }
			propManager.UnpickedPropListLock.EnterReadLock(); try { gameObjList.AddRange(propManager.UnpickedPropList); } finally { propManager.UnpickedPropListLock.ExitReadLock(); }
			return gameObjList;
		}

		/// <summary>
		/// 获取玩家，可以获取已经下场的玩家
		/// </summary>
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
			return teamList[(int)teamID].GetPlayerIDs();
		}

		/// <summary>
		/// 扔道具
		/// </summary>
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
			teamList = new List<Team>();
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

using System;
using System.Collections;
using System.Collections.Generic;
using THUnity2D;
using System.Threading;
using System.Threading.Tasks;
using GameEngine;
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
		public Map.ColorType TeamToColor(long teamID)
		{
			return (Map.ColorType)(teamID + 1L);
		}
		public long ColorToTeam(Map.ColorType color)
		{
			return (long)color - 1L;
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
			gameMap.SetCellColor(cellX, cellY, TeamToColor(playerInitInfo.teamID));

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
							if (gameMap.GetCellColor(cellX, cellY) == TeamToColor(newPlayer.TeamID)) newPlayer.AddBulletNum();

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
			if (playerToMove != null) moveEngine.MoveObj(playerToMove, moveTimeInMilliseconds, moveDirection);
		}

		//碰撞后处理，返回是否已经销毁该对象
		private bool OnCollision(GameObject obj, GameObject collisionObj, Vector moveVec)
		{
			if (collisionObj is Mine)
			{
				ActivateMine((Character)obj, (Mine)collisionObj);
				return false;
			}
			else if (obj is Character)      //如果是人主动碰撞
			{
				return false;
			}
			else if (obj is Bullet)
			{
				//如果越界，爆炸
				if (collisionObj is OutOfBoundBlock)
				{
					BulletBomb((Bullet)obj, null);
					return true;
				}

				if (obj.IsRigid)        //CheckCollision保证了collisionObj不可能是BirthPoint
				{
					BulletBomb((Bullet)obj, collisionObj); return true;
				}

				return false;
			}
			return false;
		}

		private void BulletBomb(Bullet bullet, GameObject? objBeingShot)
		{
			GameObject.Debug(bullet, " bombed!");
			/*子弹要爆炸时的行为*/

			bullet.CanMove = false;
			/*从列表中删除*/
			gameMap.ObjListLock.EnterWriteLock();
			try
			{
				foreach (GameObject obj in gameMap.ObjList)
				{
					if (obj.ID == bullet.ID)
					{
						//
						gameMap.ObjList.Remove(obj);
						break;
					}
				}
			}
			finally { gameMap.ObjListLock.ExitWriteLock(); }

			//攻击一个玩家函数
			Action<Character> BombOnePlayer = (Character playerBeingShot) =>
			{
				if (playerBeingShot.TeamID != bullet.Parent.TeamID)     //如果击中的不是队友
				{
					playerBeingShot.BeAttack(bullet.AP, bullet.HasSpear, bullet.Parent);
					if (playerBeingShot.HP <= 0)                //如果打死了
					{
						//人被打死时会停滞1秒钟，停滞的时段内暂从列表中删除，以防止其产生任何动作（行走、攻击等）
						playerBeingShot.CanMove = false;
						playerBeingShot.IsResetting = true;
						gameMap.PlayerListLock.EnterWriteLock();
						try
						{
							gameMap.PlayerList.Remove(playerBeingShot);
						}
						finally { gameMap.PlayerListLock.ExitWriteLock(); }
						playerBeingShot.Reset();

						bullet.Parent.AddScore(Map.Constant.addScoreWhenKillOnePlayer);  //给击杀者加分

						new Thread
							(() =>
							{

								Thread.Sleep(Map.Constant.deadRestoreTime);

								playerBeingShot.AddShield(Map.Constant.shieldTimeAtBirth);  //复活加个盾

								gameMap.PlayerListLock.EnterWriteLock();
								try
								{
									gameMap.PlayerList.Add(playerBeingShot);
								}
								finally { gameMap.PlayerListLock.ExitWriteLock(); }

								if (gameMap.Timer.IsGaming)
								{
									playerBeingShot.CanMove = true;
								}
								playerBeingShot.IsResetting = false;
							}
							)
						{ IsBackground = true }.Start();
					}
				}
			};

			if (objBeingShot != null)
			{
				if (objBeingShot is Character)  //如果击中了玩家
				{
					BombOnePlayer((Character)objBeingShot);
				}
				else if (objBeingShot is Bullet)        //如果被击中的是另一个子弹，把它爆掉
				{
					new Thread(() => { BulletBomb((Bullet)objBeingShot, null); }) { IsBackground = true }.Start();
				}
			}

			/*改变地图颜色*/

			int cellX = Map.Constant.GridToCellX(bullet.Position), cellY = Map.Constant.GridToCellY(bullet.Position);
			var colorRange = bullet.GetColorRange();

			/*哪些颜色不能够被改变*/
			bool[,] cannotColor = new bool[gameMap.Rows, gameMap.Cols];

			gameMap.ObjListLock.EnterReadLock();
			try
			{
				foreach (GameObject obj in gameMap.ObjList)
				{
					if (obj.IsRigid && (obj is Wall || obj is BirthPoint))
					{
						cannotColor[Map.Constant.GridToCellX(obj.Position), Map.Constant.GridToCellY(obj.Position)] = true;
					}
				}
			}
			finally { gameMap.ObjListLock.ExitReadLock(); }

			foreach (var pos in colorRange)
			{
				int colorCellX = cellX + pos.x, colorCellY = cellY + pos.y;
				if (colorCellX < 0 || colorCellX >= gameMap.Rows || colorCellY < 0 || colorCellY >= gameMap.Cols) continue;
				if (!cannotColor[colorCellX, colorCellY])
				{
					gameMap.SetCellColor(colorCellX, colorCellY, TeamToColor(bullet.Parent.TeamID));
				}
			}

			var attackRange = bullet.GetAttackRange();
			for (int i = 0; i < attackRange.GetLength(0); ++i)  //化为实际格子坐标
			{
				attackRange[i].x += cellX;
				attackRange[i].y += cellY;
			}
			ArrayList willBeAttacked = new ArrayList();
			gameMap.PlayerListLock.EnterReadLock();
			try
			{
				foreach (Character player in gameMap.PlayerList)
				{
					int playerCellX = Map.Constant.GridToCellX(player.Position), playerCellY = Map.Constant.GridToCellY(player.Position);
					foreach (var pos in attackRange)
					{
						if (pos.x == playerCellX && pos.y == playerCellY && !object.ReferenceEquals(player, objBeingShot)) { willBeAttacked.Add(player); }
					}
				}
			}
			finally { gameMap.PlayerListLock.ExitReadLock(); }
			foreach (Character player in willBeAttacked)
			{
				BombOnePlayer(player);
			}
			willBeAttacked.Clear();

			gameMap.ObjListLock.EnterReadLock();
			try
			{
				foreach (Obj obj in gameMap.ObjList)
				{
					if (obj.IsRigid && obj is Bullet)
					{
						int objCellX = Map.Constant.GridToCellX(obj.Position), objCellY = Map.Constant.GridToCellY(obj.Position);
						foreach (var pos in attackRange)
						{
							if (pos.x == objCellX && pos.y == objCellY && !object.ReferenceEquals(obj, objBeingShot)) { willBeAttacked.Add(obj); }
						}
					}
				}
			}
			finally { gameMap.ObjListLock.ExitReadLock(); }
			foreach (Bullet beAttakedBullet in willBeAttacked)
			{
				new Thread(() => { BulletBomb(beAttakedBullet, null); }) { IsBackground = true }.Start();
			}
			willBeAttacked.Clear();
		}

		//攻击
		public bool Attack(long playerID, int timeInMilliseconds, double angle)
		{
			if (!gameMap.Timer.IsGaming) return false;
			Character? playerWillAttack = gameMap.FindPlayer(playerID);

			if (playerWillAttack != null)
			{
				if (!playerWillAttack.IsAvailable) return false;
				if (playerWillAttack.Attack())
				{
					Bullet newBullet = new Bullet(
						playerWillAttack.Position + new XYPosition((int)(Map.Constant.numOfGridPerCell * Math.Cos(angle)), (int)(Map.Constant.numOfGridPerCell * Math.Sin(angle))),
						Map.Constant.bulletRadius, Map.Constant.basicBulletMoveSpeed, playerWillAttack.bulletType, playerWillAttack.AP, playerWillAttack.HasSpear);

					newBullet.Parent = playerWillAttack;

					switch (playerWillAttack.bulletType)
					{
						case BulletType.Bullet0:
						case BulletType.Bullet6:
							timeInMilliseconds = int.MaxValue;
							break;
						case BulletType.Bullet5:
							timeInMilliseconds = 0;
							angle = playerWillAttack.FacingDirection;
							break;
						case BulletType.Bullet3:        //不断检测它所位于的格子，并将其染色
							timeInMilliseconds = int.MaxValue;
							Task.Run
								(
									() =>
									{
										for (int i = 0; i < 50 && !newBullet.CanMove; ++i)      //等待子弹开始移动，最多等待50次
									{
											Thread.Sleep(1000 / Map.Constant.numOfStepPerSecond);
										}

										new FrameRateTaskExecutor<int>
										(
											() => newBullet.CanMove,
											() =>
											{
												int cellX = Map.Constant.GridToCellX(newBullet.Position), cellY = Map.Constant.GridToCellY(newBullet.Position);

												if (cellX >= 0 && cellX < gameMap.Rows && cellY >= 0 && cellY < gameMap.Cols)
												{
													bool canColor = true;
													gameMap.ObjListLock.EnterReadLock();
													try
													{
														foreach (GameObject obj in gameMap.ObjList)
														{
															if (obj.IsRigid
															&& Map.Constant.GridToCellX(obj.Position) == cellX
															&& Map.Constant.GridToCellY(obj.Position) == cellY
															&& (obj is Wall || obj is BirthPoint))
															{
																canColor = false;
																break;
															}
														}
													}
													finally { gameMap.ObjListLock.ExitReadLock(); }

													if (canColor)
													{
														gameMap.SetCellColor(cellX, cellY, TeamToColor(newBullet.Parent.TeamID));
													}
												}
											},
											1000 / Map.Constant.numOfStepPerSecond,
											() => 0
										).Start();
									}
								);

							break;
					}

					gameMap.ObjListLock.EnterWriteLock(); try { gameMap.ObjList.Add(newBullet); } finally { gameMap.ObjListLock.ExitWriteLock(); }

					newBullet.CanMove = true;
					moveEngine.MoveObj(newBullet, timeInMilliseconds, angle);
					return true;
				}
			}

			return false;
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

		private void ActivateMine(Character player, Mine mine)
		{
			gameMap.ObjListLock.EnterWriteLock();
			try { gameMap.ObjList.Remove(mine); }
			catch { }
			finally { gameMap.ObjListLock.ExitWriteLock(); }

			switch (mine.GetPropType())
			{
				case PropType.Dirt:
					player.AddMoveSpeed(Map.Constant.dirtMoveSpeedDebuff, Map.Constant.buffPropTime);
					break;
				case PropType.Attenuator:
					player.AddAP(Map.Constant.attenuatorAtkDebuff, Map.Constant.buffPropTime);
					break;
				case PropType.Divider:
					player.ChangeCD(Map.Constant.dividerCdDiscount, Map.Constant.buffPropTime);
					break;
			}
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
			return gameMap.GetColorArea(TeamToColor(teamID));
		}

		private MoveEngine moveEngine;
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
			moveEngine = new MoveEngine
			(
				gameMap: gameMap,
				OnCollision: OnCollision,
				EndMove: obj =>
				{
					GameObject.Debug(obj, " end move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64);
					if (obj is Bullet) BulletBomb((Bullet)obj, null);
				}
			);

			propManager = new PropManager(gameMap, moveEngine);
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

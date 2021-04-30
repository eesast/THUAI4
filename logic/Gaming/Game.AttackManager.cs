using GameEngine;
using System;
using System.Collections;
using System.Threading;
using System.Threading.Tasks;
using THUnity2D;
using Timothy.FrameRateTask;

namespace Gaming
{
	public partial class Game
	{
		AttackManager attackManager;

		class AttackManager
		{
			public bool Attack(Character playerWillAttack, int timeInMilliseconds, double angle)
			{
				if (!playerWillAttack.IsAvailable) return false;
				if (playerWillAttack.Attack())
				{
					Bullet newBullet = new Bullet(
						playerWillAttack.Position + new XYPosition((int)(Map.Constant.numOfGridPerCell * Math.Cos(angle)), (int)(Map.Constant.numOfGridPerCell * Math.Sin(angle))),
						Map.Constant.bulletRadius, Map.Constant.basicBulletMoveSpeed, playerWillAttack.bulletType, playerWillAttack.AP, playerWillAttack.HasSpear);

					// 由于子弹的出发点位于人物面前的亦歌位置，所以子弹移动时间需要扣减
					if (timeInMilliseconds <= Map.Constant.numOfGridPerCell * 1000 / newBullet.MoveSpeed) timeInMilliseconds = 0;
					else timeInMilliseconds -= Map.Constant.numOfGridPerCell * 1000 / newBullet.MoveSpeed;

					newBullet.Parent = playerWillAttack;

					switch (playerWillAttack.bulletType)
					{
						case BulletType.Bullet0:
						case BulletType.Bullet6:
							timeInMilliseconds = int.MaxValue;
							break;
						case BulletType.Bullet5:
							timeInMilliseconds = 0;
							break;
						case BulletType.Bullet3:        //不断检测它所位于的格子，并将其染色
							timeInMilliseconds = int.MaxValue;
							new Thread
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
														gameMap.SetCellColor(cellX, cellY, Map.TeamToColor(newBullet.Parent.TeamID));
													}
												}
											},
											1000 / Map.Constant.numOfStepPerSecond,
											() => 0
										).Start();
									}
								)
							{ IsBackground = true }.Start();

							break;
					}

					gameMap.ObjListLock.EnterWriteLock(); try { gameMap.ObjList.Add(newBullet); } finally { gameMap.ObjListLock.ExitWriteLock(); }
					
					newBullet.CanMove = true;
					moveEngine.MoveObj(newBullet, timeInMilliseconds, angle);
					return true;
				}
				return false;
			}

			/// <summary>
			/// 攻击一个玩家
			/// </summary>
			/// <param name="bullet">攻击的子弹</param>
			/// <param name="playerBeingShot">被打到的玩家</param>
			private void BombOnePlayer(Bullet bullet, Character playerBeingShot)
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

			/// <summary>
			/// 爆掉子弹
			/// </summary>
			/// <param name="bullet">要爆炸的子弹</param>
			/// <param name="objBeingShot">子弹打到的物体，如果为null，则未打到物体便爆炸，例如越界或被其他子弹引爆</param>
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

				if (objBeingShot != null)
				{
					if (objBeingShot is Character)  //如果击中了玩家
					{
						BombOnePlayer(bullet, (Character)objBeingShot);
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
						gameMap.SetCellColor(colorCellX, colorCellY, Map.TeamToColor(bullet.Parent.TeamID));
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
					BombOnePlayer(bullet, player);
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

			Map gameMap;
			MoveEngine moveEngine;

			public AttackManager(Map gameMap)
			{
				this.gameMap = gameMap;
				this.moveEngine = new MoveEngine
					(
						gameMap: gameMap,
						OnCollision: (obj, collisionObj, moveVec) =>
						{
							BulletBomb((Bullet)obj, collisionObj);
							return MoveEngine.AfterCollision.Destroyed;
						},
						EndMove: obj =>
						{
							GameObject.Debug(obj, " end move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64);
							BulletBomb((Bullet)obj, null);
						},
						IgnoreCollision: (obj, collisionObj) =>
						{
							if (collisionObj is BirthPoint || collisionObj is Mine) return true;    // 子弹不和出生点与地雷碰撞
							return false;
						}
					);
			}
		}
	}
}

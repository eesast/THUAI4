using GameEngine;
using System;
using System.Collections.Generic;
using System.Threading;
using THUnity2D;

namespace Gaming
{
	public partial class Game
	{
		private PropManager propManager;

		private class PropManager
		{
			private LinkedList<Prop> unpickedPropList;          // 尚未捡起的道具列表
			private ReaderWriterLockSlim unpickedPropListLock;
			public LinkedList<Prop> UnpickedPropList => unpickedPropList;
			public ReaderWriterLockSlim UnpickedPropListLock => unpickedPropListLock;

			private bool IsProducingProp { get; set; } = false;
			private object isPropducingPropLock = new object();

			public bool StartProducing()
			{
				lock (isPropducingPropLock)
				{
					if (IsProducingProp) return false;
				}

				unpickedPropListLock.EnterWriteLock();
				try
				{
					unpickedPropList.Clear();
				}
				finally { unpickedPropListLock.ExitWriteLock(); }

				//开始产生道具

				new Thread
				(
					() =>
					{
						while (!gameMap.Timer.IsGaming) Thread.Sleep(1000);
						while (gameMap.Timer.IsGaming)
						{
							var beginTime = Environment.TickCount64;
							ProduceOneProp();
							var endTime = Environment.TickCount64;
							var deltaTime = endTime - beginTime;
							if (deltaTime <= Map.Constant.producePropTimeInterval)
							{
								Thread.Sleep(Map.Constant.producePropTimeInterval - (int)deltaTime);
							}
							else
							{
								Console.WriteLine("In Function StartGame: The computer runs too slow that it cannot produce one prop in the given time!");
							}
						}
					}
				)
				{ IsBackground = true }.Start();

				return true;
			}

			private void ProduceOneProp()
			{
				Random r = new Random((int)Environment.TickCount64);
				XYPosition newPropPos = new XYPosition();
				while (true)
				{
					newPropPos.x = r.Next(0, gameMap.Rows * Map.Constant.numOfGridPerCell);
					newPropPos.y = r.Next(0, gameMap.Cols * Map.Constant.numOfGridPerCell);
					int cellX = Map.Constant.GridToCellX(newPropPos), cellY = Map.Constant.GridToCellY(newPropPos);
					bool canLayProp = true;
					gameMap.ObjListLock.EnterReadLock();
					try
					{
						foreach (GameObject obj in gameMap.ObjList)
						{
							if (cellX == Map.Constant.GridToCellX(obj.Position) && cellY == Map.Constant.GridToCellY(obj.Position) && (obj is Wall || obj is BirthPoint))
							{
								canLayProp = false;
								break;
							}
						}
					}
					finally { gameMap.ObjListLock.ExitReadLock(); }
					if (canLayProp)
					{
						newPropPos = Map.Constant.CellToGrid(cellX, cellY);
						break;
					}
				}

				PropType propType = (PropType)r.Next(Prop.MinPropTypeNum, Prop.MaxPropTypeNum + 1);

				Prop? newProp = null;
				switch (propType)
				{
					case PropType.Bike: newProp = new Bike(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Amplifier: newProp = new Amplifier(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.JinKeLa: newProp = new JinKeLa(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Rice: newProp = new Rice(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Shield: newProp = new Shield(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Totem: newProp = new Totem(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Spear: newProp = new Spear(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Dirt: newProp = new Dirt(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Attenuator: newProp = new Attenuator(newPropPos, Map.Constant.unpickedPropRadius); break;
					case PropType.Divider: newProp = new Divider(newPropPos, Map.Constant.unpickedPropRadius); break;
				}
				if (newProp != null)
				{
					unpickedPropListLock.EnterWriteLock();
					try { unpickedPropList.AddLast(newProp); }
					finally { unpickedPropListLock.ExitWriteLock(); }
					newProp.CanMove = true;
				}
			}

			public Prop? PickProp(Character player, PropType propType)
			{
				if (!player.IsAvailable) return null;

				lock (player.propLock)
				{
					while (player.IsModifyingProp) Thread.Sleep(1);
					player.IsModifyingProp = true;
				}

				int cellX = Map.Constant.GridToCellX(player.Position), cellY = Map.Constant.GridToCellY(player.Position);

#if DEBUG
				Console.WriteLine("Try picking: {0} {1} Type: {2}", cellX, cellY, (int)propType);
#endif

				Prop? prop = null;
				unpickedPropListLock.EnterWriteLock();
				try
				{
					for (LinkedListNode<Prop>? propNode = unpickedPropList.First; propNode != null; propNode = propNode.Next)
					{
#if DEBUG
						Console.WriteLine("Picking: Now check type: {0}", (int)propNode.Value.GetPropType());
#endif

						if (propNode.Value.GetPropType() != propType || propNode.Value.IsMoving) continue;
						int cellXTmp = Map.Constant.GridToCellX(propNode.Value.Position), cellYTmp = Map.Constant.GridToCellY(propNode.Value.Position);

#if DEBUG
						Console.WriteLine("Ready to pick: {0} {1}, {2} {3}", cellX, cellY, cellXTmp, cellYTmp);
#endif

						if (cellXTmp == cellX && cellYTmp == cellY)
						{
							prop = propNode.Value;
							unpickedPropList.Remove(propNode);
							break;
						}
					}
				}
				finally { unpickedPropListLock.ExitWriteLock(); }

				if (prop != null)
				{
					player.HoldProp = prop;
					prop.Parent = player;
				}

				return prop;

			}

			public void ThrowProp(Character player, int moveTimeInMilliseconds, double angle)
			{
				if (!player.IsAvailable) return;
				Prop? oldProp = player.UseProp();
				if (oldProp == null) return;
				oldProp.ResetPosition(player.Position);
				oldProp.ResetMoveSpeed(Map.Constant.thrownPropMoveSpeed);

				moveEngine.MoveObj(oldProp, moveTimeInMilliseconds, angle);

				unpickedPropListLock.EnterWriteLock();
				try { unpickedPropList.AddLast(oldProp); }
				finally { unpickedPropListLock.ExitWriteLock(); }
			}

			public void RemoveProp(Prop prop)
			{
				unpickedPropListLock.EnterWriteLock();
				try { unpickedPropList.Remove(prop); }
				catch { }
				finally { unpickedPropListLock.ExitWriteLock(); }
			}

			public void UseProp(Character player)
			{

				if (!player.IsAvailable) return;

				lock (player.propLock)
				{
					while (player.IsModifyingProp) Thread.Sleep(1);
					player.IsModifyingProp = true;
				}

				Prop? prop = player.HoldProp;
				player.HoldProp = null;

				player.IsModifyingProp = false;

				if (prop != null)
				{
					if (prop is Buff)
					{
						switch (prop.GetPropType())
						{
							case PropType.Bike:
								player.AddMoveSpeed(Map.Constant.bikeMoveSpeedBuff, Map.Constant.buffPropTime);
								break;
							case PropType.Amplifier:
								player.AddAP(Map.Constant.amplifierAtkBuff, Map.Constant.buffPropTime);
								break;
							case PropType.JinKeLa:
								player.ChangeCD(Map.Constant.jinKeLaCdDiscount, Map.Constant.buffPropTime);
								break;
							case PropType.Rice:
								player.AddHp(Map.Constant.riceHpAdd);
								break;
							case PropType.Shield:
								player.AddShield(Map.Constant.shieldTime);
								break;
							case PropType.Totem:
								player.AddTotem(Map.Constant.totemTime);
								break;
							case PropType.Spear:
								player.AddSpear(Map.Constant.spearTime);
								break;
						}
					}
					else if (prop is Mine)
					{
						Mine mine = (Mine)prop;
						mine.SetLaid(player.Position);
						new Thread
							(
								() =>
								{
									gameMap.ObjListLock.EnterWriteLock();
									try
									{
										gameMap.ObjList.Add(mine);
									}
									finally { gameMap.ObjListLock.ExitWriteLock(); }

									Thread.Sleep(Map.Constant.mineTime);

									gameMap.ObjListLock.EnterWriteLock();
									try { gameMap.ObjList.Remove(mine); }
									catch { }
									finally { gameMap.ObjListLock.ExitWriteLock(); }
								}
							)
						{ IsBackground = true }.Start();
					}
				}
			}

			private Map gameMap;
			private MoveEngine moveEngine;

			public PropManager(Map gameMap)
			{
				this.gameMap = gameMap;
				this.moveEngine = new MoveEngine
				(
					gameMap: gameMap,
					OnCollision: (obj, collisionObj, moveVec) =>
					{
						//越界，清除出游戏
						RemoveProp((Prop)obj);
						return MoveEngine.AfterCollision.Destroyed;
					},
					EndMove: obj =>
					{
						GameObject.Debug(obj, " end move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64);
					}
				);
				unpickedPropList = new LinkedList<Prop>();
				unpickedPropListLock = new ReaderWriterLockSlim();
			}
		}
	}
}

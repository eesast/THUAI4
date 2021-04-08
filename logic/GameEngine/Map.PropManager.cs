using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using THUnity2D;

namespace GameEngine
{
	public partial class Map
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

				Task.Run
				(
					() =>
					{
						Map parentMap = this.parentMap;		// 记录父地图，防止游戏结束时释放父地图导致对 null 解引用
						if (parentMap == null) return;
						while (!parentMap.IsGaming) Thread.Sleep(1000);
						while (parentMap.IsGaming)
						{
							var beginTime = Environment.TickCount64;
							ProduceOneProp();
							var endTime = Environment.TickCount64;
							var deltaTime = endTime - beginTime;
							if (deltaTime <= Constant.producePropTimeInterval)
							{
								Thread.Sleep(Constant.producePropTimeInterval - (int)deltaTime);
							}
							else
							{
								Console.WriteLine("In Function StartGame: The computer runs too slow that it cannot produce one prop in the given time!");
							}
						}
					}
				);

				return true;
			}

			private void ProduceOneProp()
			{
				Random r = new Random((int)Environment.TickCount64);
				XYPosition newPropPos = new XYPosition();
				while (true)
				{
					Map parentMap = this.parentMap;     // 记录父地图，防止游戏结束时突然释放父地图导致异常
					if (parentMap == null) return;
					newPropPos.x = r.Next(0, parentMap.Rows * Constant.numOfGridPerCell);
					newPropPos.y = r.Next(0, parentMap.Cols * Constant.numOfGridPerCell);
					int cellX = Constant.GridToCellX(newPropPos), cellY = Constant.GridToCellY(newPropPos);
					bool canLayProp = true;
					parentMap.objListLock.EnterReadLock();
					try
					{
						foreach (GameObject obj in parentMap.objList)
						{
							if (cellX == Constant.GridToCellX(obj.Position) && cellY == Constant.GridToCellY(obj.Position) && (obj is Wall || obj is BirthPoint))
							{
								canLayProp = false;
								break;
							}
						}
					}
					finally { parentMap.objListLock.ExitReadLock(); }
					if (canLayProp)
					{
						newPropPos = Constant.CellToGrid(cellX, cellY);
						break;
					}
				}

				PropType propType = (PropType)r.Next(Prop.MinPropTypeNum, Prop.MaxPropTypeNum + 1);

				Prop? newProp = null;
				switch (propType)
				{
					case PropType.Bike: newProp = new Bike(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Amplifier: newProp = new Amplifier(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.JinKeLa: newProp = new JinKeLa(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Rice: newProp = new Rice(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Shield: newProp = new Shield(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Totem: newProp = new Totem(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Spear: newProp = new Spear(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Dirt: newProp = new Dirt(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Attenuator: newProp = new Attenuator(newPropPos, Constant.unpickedPropRadius); break;
					case PropType.Divider: newProp = new Divider(newPropPos, Constant.unpickedPropRadius); break;
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

				int cellX = Constant.GridToCellX(player.Position), cellY = Constant.GridToCellY(player.Position);

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
						int cellXTmp = Constant.GridToCellX(propNode.Value.Position), cellYTmp = Constant.GridToCellY(propNode.Value.Position);

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
				oldProp.ResetMoveSpeed(Constant.thrownPropMoveSpeed);

				Map parentMap = this.parentMap;
				if (parentMap == null) return;
				parentMap.moveMagager.MoveObj(oldProp, moveTimeInMilliseconds, angle);

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
								player.AddMoveSpeed(Constant.bikeMoveSpeedBuff, Constant.buffPropTime);
								break;
							case PropType.Amplifier:
								player.AddAP(Constant.amplifierAtkBuff, Constant.buffPropTime);
								break;
							case PropType.JinKeLa:
								player.ChangeCD(Constant.jinKeLaCdDiscount, Constant.buffPropTime);
								break;
							case PropType.Rice:
								player.AddHp(Constant.riceHpAdd);
								break;
							case PropType.Shield:
								player.AddShield(Constant.shieldTime);
								break;
							case PropType.Totem:
								player.AddTotem(Constant.totemTime);
								break;
							case PropType.Spear:
								player.AddSpear(Constant.spearTime);
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
									parentMap.objListLock.EnterWriteLock();
									try
									{
										parentMap.objList.Add(mine);
									}
									finally { parentMap.objListLock.ExitWriteLock(); }

									Thread.Sleep(Constant.mineTime);

									parentMap.objListLock.EnterWriteLock();
									try { parentMap.objList.Remove(mine); }
									catch { }
									finally { parentMap.objListLock.ExitWriteLock(); }
								}
							)
						{ IsBackground = true }.Start();
					}
				}
			}

			private Map parentMap;

			public PropManager(Map parentMap)
			{
				this.parentMap = parentMap;
				unpickedPropList = new LinkedList<Prop>();
				unpickedPropListLock = new ReaderWriterLockSlim();
			}

			// 释放父地图，防止内存泄漏
			~PropManager()
			{
				parentMap = null;
			}
		}
	}
}

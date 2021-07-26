using System;
using System.Collections.Generic;
using System.Threading;
using THUnity2D.Interfaces;
using THUnity2D.Utility;

namespace GameEngine
{
	internal class CollisionChecker
	{
		/// <summary>
		/// 碰撞检测，如果这样行走是否会与之碰撞，返回与之碰撞的物体
		/// </summary>
		/// <param name="obj">移动的物体</param>
		/// <param name="moveVec">移动的位移向量</param>
		/// <returns>和它碰撞的物体</returns>
		public IGameObj? CheckCollision(IMovable obj, Vector moveVec)
		{
			XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);

			if (!obj.IsRigid)
			{
				if (gameMap.OutOfBound(obj)) return gameMap.GetOutOfBoundBlock(nextPos);
				return null;
			}

			//在某列表中检查碰撞
			Func<IEnumerable<IGameObj>, ReaderWriterLockSlim, IGameObj?> CheckCollisionInList =
				(IEnumerable<IGameObj> lst, ReaderWriterLockSlim listLock) =>
				{
					IGameObj? collisionObj = null;
					listLock.EnterReadLock();
					try
					{
						foreach (var listObj in lst)
						{
							if (obj.WillCollideWith(listObj, nextPos))
							{
								collisionObj = listObj;
								break;
							}
						}
					}
					finally { listLock.ExitReadLock(); }
					return collisionObj;
				};

			IGameObj? collisionObj = null;
			foreach (var list in lists)
			{
				if ((collisionObj = CheckCollisionInList(list.Item1, list.Item2)) != null)
				{
					return collisionObj;
				}
			}

			//如果越界，则与越界方块碰撞
			if (gameMap.OutOfBound(obj))
			{
				return gameMap.GetOutOfBoundBlock(nextPos);
			}

			return null;
		}

		/// <summary>
		/// 寻找最大可能移动距离
		/// </summary>
		/// <param name="obj">移动物体，默认obj.Rigid为true</param>
		/// <param name="nextPos">下一步要到达的位置</param>
		/// <param name="moveVec">移动的位移向量，默认与nextPos协调</param>
		/// <returns>最大可能的移动距离</returns>
		public uint FindMax(IMovable obj, XYPosition nextPos, Vector moveVec)
		{
			uint maxLen = uint.MaxValue;
			uint tmpMax;
			foreach (var listWithLock in lists)
			{
				var lst = listWithLock.Item1;
				var listLock = listWithLock.Item2;
				listLock.EnterReadLock();
				try
				{
					foreach (IGameObj listObj in lst)
					{
						//如果再走一步发生碰撞
						if (obj.WillCollideWith(listObj, nextPos))
						{
							switch (listObj.Shape)  //默认obj为圆形
							{
								case ShapeType.Circle:
									{
										//计算两者之间的距离
										int orgDeltaX = listObj.Position.x - obj.Position.x;
										int orgDeltaY = listObj.Position.y - obj.Position.y;
										double mod = Math.Sqrt((long)orgDeltaX * orgDeltaX + (long)orgDeltaY * orgDeltaY);

										if (mod == 0.0)     //如果两者重合
										{
											tmpMax = 0;
										}
										else
										{
											Vector2 relativePosUnitVector = new Vector2(orgDeltaX / mod, orgDeltaY / mod);  //相对位置的单位向量
											Vector2 moveUnitVector = new Vector2(Math.Cos(moveVec.angle), Math.Sin(moveVec.angle)); //运动方向的单位向量
											if (relativePosUnitVector * moveUnitVector <= 0) continue;      //如果它们的内积小于零，即反向，那么不会发生碰撞
										}

										double tmp = mod - obj.Radius - listObj.Radius;
										if (tmp <= 0)           //如果它们已经贴合了，那么不能再走了
										{
											tmpMax = 0;
										}
										else
										{
											//计算最多能走的距离
											tmp = tmp / Math.Cos(Math.Atan2(orgDeltaY, orgDeltaX) - moveVec.angle);
											if (tmp < 0 || tmp > uint.MaxValue || tmp == double.NaN)
											{
												tmpMax = uint.MaxValue;
											}
											else tmpMax = (uint)tmp;
										}
										break;
									}
								case ShapeType.Square:
									{
										//如果当前已经贴合，那么不能再行走了
										if (obj.WillCollideWith(listObj, obj.Position)) tmpMax = 0;
										else
										{
											//二分查找最大可能移动距离
											int left = 0, right = (int)moveVec.length;
											while (left < right - 1)
											{
												int mid = (right - left) / 2 + left;
												if (obj.WillCollideWith(listObj, obj.Position + new XYPosition((int)(mid * Math.Cos(moveVec.angle)), (int)(mid * Math.Sin(moveVec.angle)))))
												{
													right = mid;
												}
												else left = mid;
											}
											tmpMax = (uint)left;
										}
										break;
									}
								default:
									tmpMax = int.MaxValue;
									break;
							}

							if (tmpMax < maxLen) maxLen = tmpMax;
						}
					}
				}
				finally { listLock.ExitReadLock(); }
			}
			return maxLen;
		}

		IMap gameMap;
		private Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>[] lists;

		public CollisionChecker(IMap gameMap)
		{
			this.gameMap = gameMap;
			lists = new Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>[]
			{
				new Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>(gameMap.ObjList, gameMap.ObjListLock),
				new Tuple<IEnumerable<IGameObj>, ReaderWriterLockSlim>(gameMap.PlayerList, gameMap.PlayerListLock)
			};
		}
	}
}

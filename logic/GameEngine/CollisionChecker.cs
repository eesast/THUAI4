using System;
using System.Collections;
using System.Threading;
using THUnity2D;

namespace GameEngine
{
	internal static class CollisionChecker
	{
		//检查obj下一步位于nextPos时是否会与listObj碰撞
		private static bool WillCollide(GameObject obj, GameObject listObj, XYPosition nextPos)
		{
			if (!listObj.IsRigid || listObj.ID == obj.ID) return false; //不检查自己和非刚体

			if (listObj is BirthPoint)          //如果是出生点，那么除了自己以外的其他玩家需要检查碰撞
			{
				//如果是角色并且出生点不是它的出生点，需要检查碰撞，否则不检查碰撞；
				//下面的条件是obj is Character && !object.ReferenceEquals(((BirthPoint)listObj).Parent, obj)求非得结果
				if (!(obj is Character) || object.ReferenceEquals(((BirthPoint)listObj).Parent, obj)) return false;
			}

			if (listObj is Mine)
			{
				if (!(obj is Character)) return false;  //非人物不需要检查碰撞
				if (((Mine)listObj).Parent.TeamID == ((Character)obj).TeamID) return false;     //同组的人不会触发地雷
			}

			int deltaX = Math.Abs(nextPos.x - listObj.Position.x), deltaY = Math.Abs(nextPos.y - listObj.Position.y);

			//默认obj是圆形的，因为能移动的物体目前只有圆形；且会移动的道具尚未被捡起，其形状没有意义，可默认为圆形

			switch (listObj.Shape)
			{
				case GameObject.ShapeType.Circle:       //圆与圆碰撞
					{
						return (long)deltaX * deltaX + (long)deltaY * deltaY < ((long)obj.Radius + listObj.Radius) * ((long)obj.Radius + listObj.Radius);
					}
				case GameObject.ShapeType.Sqare:        //圆与正方形碰撞
					{
						if (deltaX >= listObj.Radius + obj.Radius || deltaY >= listObj.Radius + obj.Radius) return false;
						if (deltaX < listObj.Radius || deltaY < listObj.Radius) return true;
						return (long)(deltaX - listObj.Radius) * (long)(deltaY - listObj.Radius) < (long)obj.Radius * (long)obj.Radius;
						////return !(deltaX >= listObj.Radius + obj.Radius || deltaY >= listObj.Radius + obj.Radius) && ((deltaX < listObj.Radius || deltaY < listObj.Radius) || ((long)(deltaX - listObj.Radius) * (long)(deltaY - listObj.Radius) < (long)obj.Radius * (long)obj.Radius));
					}
			}
			return false;
		}


		//碰撞检测，如果这样行走是否会与之碰撞，返回与之碰撞的物体
		public static GameObject? CheckCollision(GameObject obj, Vector moveVec, Func<GameObject, bool> outOfBound, Tuple<ArrayList, ReaderWriterLockSlim>[] lists)
		{
			XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);

			//在某列表中检查碰撞
			Func<ArrayList, ReaderWriterLockSlim, GameObject> CheckCollisionInList =
				(ArrayList lst, ReaderWriterLockSlim listLock) =>
				{
					GameObject? collisionObj = null;
					listLock.EnterReadLock();
					try
					{
						foreach (GameObject listObj in lst)
						{
							if (WillCollide(obj, listObj, nextPos))
							{
								collisionObj = listObj;
								break;
							}
						}
					}
					finally { listLock.ExitReadLock(); }

					//如果越界，则与越界方块碰撞
					if (collisionObj == null && outOfBound(obj))
					{
						collisionObj = new OutOfBoundBlock(nextPos);
					}
					return collisionObj;
				};

			GameObject collisionObj = null;
			foreach (var list in lists)
			{
				if ((collisionObj = CheckCollisionInList(list.Item1, list.Item2)) != null)
				{
					return collisionObj;
				}
			}
			return null;
		}

		// 寻找最大可能移动距离

		public static uint FindMax(GameObject obj, XYPosition nextPos, Vector moveVec, Tuple<ArrayList, ReaderWriterLockSlim>[] lists)
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
					foreach (GameObject listObj in lst)
					{
						//如果再走一步发生碰撞
						if (WillCollide(obj, listObj, nextPos))
						{
							switch (listObj.Shape)  //默认obj为圆形
							{
								case GameObject.ShapeType.Circle:
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
								case GameObject.ShapeType.Sqare:
									{
										//如果当前已经贴合，那么不能再行走了
										if (WillCollide(obj, listObj, obj.Position)) tmpMax = 0;
										else
										{
											//二分查找最大可能移动距离
											int left = 0, right = (int)moveVec.length;
											while (left < right - 1)
											{
												int mid = (right - left) / 2 + left;
												if (WillCollide(obj, listObj, obj.Position + new XYPosition((int)(mid * Math.Cos(moveVec.angle)), (int)(mid * Math.Sin(moveVec.angle)))))
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
	}
}

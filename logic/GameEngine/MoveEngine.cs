using System;
using System.Collections;
using System.Threading;
using System.Threading.Tasks;
using THUnity2D;
using Timothy.FrameRateTask;

namespace GameEngine
{
	public class MoveEngine
	{

		//检查obj下一步位于nextPos时是否会与listObj碰撞
		private bool WillCollide(GameObject obj, GameObject listObj, XYPosition nextPos)
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
		private GameObject? CheckCollision(GameObject obj, Vector moveVec)
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
					if (collisionObj == null && gameMap.OutOfBound(obj))
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

		private uint FindMax(GameObject obj, XYPosition nextPos, Vector moveVec)
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

		private void MoveMax(GameObject obj,  Vector moveVec)
		{

			/*由于四周是墙，所以人物永远不可能与越界方块碰撞*/

			XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);

			uint maxLen = FindMax(obj, nextPos, moveVec);

			maxLen = (uint)Math.Min(maxLen, (obj.MoveSpeed / Map.Constant.numOfStepPerSecond));
			obj.Move(new Vector(moveVec.angle, maxLen));
		}

		//物体移动
		public void MoveObj(GameObject obj, int moveTime, double moveDirection)
		{
			Task.Run
			(
				() =>
				{
					lock (obj.moveLock)
					{
						if (!obj.IsAvailable) return;
						obj.IsMoving = true;     //开始移动
					}

					GameObject.Debug(obj, " begin to move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64.ToString());
					double deltaLen = 0.0;      //储存行走的误差
					Vector moveVec = new Vector(moveDirection, 0.0);
					//先转向
					if (gameMap.Timer.IsGaming && obj.CanMove) deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));     //先转向
					GameObject? collisionObj = null;

					bool isDestroyed = false;
					new FrameRateTaskExecutor<int>
					(
						() => gameMap.Timer.IsGaming && obj.CanMove && !obj.IsResetting,
						() =>
						{
							moveVec.length = obj.MoveSpeed / Map.Constant.numOfStepPerSecond + deltaLen;
							deltaLen = 0;

							//越界情况处理：如果越界，那么一定与四周的墙碰撞，在OnCollision中检测碰撞
							//缺陷：半径为0的物体检测不到越界
							//改进：如果越界，则与越界方块碰撞

							while (true)
							{
								collisionObj = CheckCollision(obj, moveVec);
								if (collisionObj == null) break;
								if (collisionObj is Mine)       //CheckCollision保证只有不同组的人物会和地雷碰撞
								{
									OnCollision(obj, collisionObj, moveVec);
								}
								else
								{
									MoveMax(obj, moveVec);
									if (OnCollision(obj, collisionObj, moveVec))
									{
										//已经被销毁

										GameObject.Debug(obj, " collide with " + collisionObj.ToString() + " and has been removed from the game.");
										isDestroyed = true;
										return false;
									}
									if (obj.IsRigid && obj is Character) moveVec.length = 0;
									break;
								}
							}

							deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));

							return true;
						},
						1000 / Map.Constant.numOfStepPerSecond,
						() =>
						{
							if (!isDestroyed)
							{
								moveVec.length = deltaLen;
								if ((collisionObj = CheckCollision(obj, moveVec)) == null)
								{
									obj.Move(moveVec);
								}
								else
								{
									OnCollision(obj, collisionObj, moveVec);
								}
								obj.IsMoving = false;        //结束移动
								EndMove(obj);
							}
							return 0;
						},
						maxTotalDuration: moveTime
					)
					{
						AllowTimeExceed = true,
						MaxTolerantTimeExceedCount = ulong.MaxValue,
						TimeExceedAction = b =>
						{
							Console.WriteLine("The computer runs so slow that the player cannot finish moving during this time!!!!!!");
						}
					}.Start();
				}
			);
		}

		private Map gameMap;
		private Tuple<ArrayList, ReaderWriterLockSlim>[] lists;
		private Func<GameObject, GameObject, Vector, bool> OnCollision;
		private Action<GameObject> EndMove;

		public MoveEngine(Map gameMap,
			Func<GameObject, GameObject, Vector, bool> OnCollision,
			Action<GameObject> EndMove
			)
		{
			this.gameMap = gameMap;
			lists = new Tuple<ArrayList, ReaderWriterLockSlim>[]
			{
				new Tuple<ArrayList, ReaderWriterLockSlim>(gameMap.ObjList, gameMap.ObjListLock),
				new Tuple<ArrayList, ReaderWriterLockSlim>(gameMap.PlayerList, gameMap.PlayerListLock)
			};
			this.OnCollision = OnCollision;
			this.EndMove = EndMove;
		}
	}
}

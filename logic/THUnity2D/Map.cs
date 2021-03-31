using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System;

namespace THUnity2D
{
	// This class, Map, is a god class, which is a terrible design.
	// Sincerely hope that you, the coder of THUAIX (X > 4) or other code readers,
	// can divide it to many small classes to take different respoinsibilities to
	// raise its maintainability, testablility and readability.
	// I strongly recommend that you can use more design-patterns well to solve this problem.
	// -- The coder of this logic of THUAI4.
	public class Map
	{
		public static class Constant
		{
			public const int numOfGridPerCell = 1000;				//每个的坐标单位数
			public const int numOfStepPerSecond = 20;				//每秒行走的步数
			public const int addScoreWhenKillOnePlayer = 0;			//击杀一名玩家后的加分
			public const int producePropTimeInterval = 20 * 1000;	//产生道具时间间隔（毫秒）

			public const int basicPlayerMoveSpeed = numOfGridPerCell * 4;
			public const int basicBulletMoveSpeed = numOfGridPerCell * 6;
			public const int thrownPropMoveSpeed = numOfGridPerCell * 8;

			public const int objMaxRadius = numOfGridPerCell / 2;
			public const int playerRadius = objMaxRadius;
			public const int wallRadius = objMaxRadius;
			public const int birthPointRadius = objMaxRadius;
			public const int bulletRadius = objMaxRadius / 2;
			public const int unpickedPropRadius = objMaxRadius;

			public const int buffPropTime = 30 * 1000;      //buff道具持续时间（毫秒）
			public const int shieldTime = buffPropTime;
			public const int totemTime = buffPropTime;
			public const int spearTime = buffPropTime;

			public const int bikeMoveSpeedBuff = numOfGridPerCell;
			public const int amplifierAtkBuff = Character.basicAp * 2;
			public const double jinKeLaCdDiscount = 0.25;
			public const int riceHpAdd = Character.basicHp / 2;

			public const int mineTime = 60 * 1000;      //地雷埋藏的持续时间
			public const int dirtMoveSpeedDebuff = numOfGridPerCell;
			public const int attenuatorAtkDebuff = amplifierAtkBuff;
			public const double dividerCdDiscount = 4.0;

			public const int deadRestoreTime = 60 * 1000;   //死亡恢复时间
			public const int shieldTimeAtBirth = 5 * 1000;  //出生时的盾牌时间

			public static XYPosition CellToGrid(int x, int y)   //求格子的中心坐标
			{
				XYPosition ret = new XYPosition(x * Constant.numOfGridPerCell + Constant.numOfGridPerCell / 2,
					y * Constant.numOfGridPerCell + Constant.numOfGridPerCell / 2);
				return ret;
			}
			public static int GridToCellX(XYPosition pos)       //求坐标所在的格子的x坐标
			{
				return pos.x / Constant.numOfGridPerCell;
			}
			public static int GridToCellY(XYPosition pos)      //求坐标所在的格子的y坐标
			{
				return pos.y / Constant.numOfGridPerCell;
			}
		}

		public enum ColorType
		{
			None = 0,
			Color1 = 1,
			Color2 = 2,
			Color3 = 3,
			Color4 = 4,
		}
		public const int maxTeamNum = 4;
		public ColorType TeamToColor(long teamID)
		{
			return (ColorType)(teamID + 1L);
		}
		public long ColorToTeam(ColorType color)
		{
			return (long)color - 1L;
		}
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

		private ColorType[,] cellColor;			//储存每格的颜色
		public ColorType[,] CellColor
		{
			get
			{
				int rows = cellColor.GetLength(0), cols = cellColor.GetLength(1);
				ColorType[,] ret = new ColorType[rows, cols];
				for (int i = 0; i < rows; ++i)
				{
					for (int j = 0; j < cols; ++j)
					{
						ret[i, j] = cellColor[i, j];
					}
				}
				return ret;
			}
		}
		public int Rows							//行数
		{
			get => cellColor.GetLength(0);
		}
		public int Cols							//列数
		{
			get => cellColor.GetLength(1);
		}

		private ArrayList objList;              //游戏对象（除了玩家外）的列表
		private ReaderWriterLockSlim objListLock = new ReaderWriterLockSlim();		//读写锁，防止foreach遍历出现冲突（若可改成无foreach遍历考虑去掉读写锁而用线程安全的ArrayList）
		private ArrayList playerList;			//玩家列表（可能要频繁通过ID查找player，但玩家最多只有8个；如果玩家更多，考虑改为SortedList）
		private ReaderWriterLockSlim playerListLock = new ReaderWriterLockSlim();
		private LinkedList<Prop> unpickedPropList;		//尚未捡起的道具列表
		private ReaderWriterLockSlim unpickedPropListLock = new ReaderWriterLockSlim();
		private ArrayList teamList;             //队伍列表
		//private object teamListLock = new object();
		private readonly Dictionary<uint, BirthPoint> birthPointList;	//出生点列表

		private readonly int numOfTeam;

		public long AddPlayer(PlayerInitInfo playerInitInfo)
		{
			if (!Team.teamExists(playerInitInfo.teamID) 
				|| !MapInfo.ValidBirthPointIdx(playerInitInfo.birthPointIdx) 
				|| birthPointList[playerInitInfo.birthPointIdx].Parent != null) return GameObject.invalidID;

			XYPosition pos = birthPointList[playerInitInfo.birthPointIdx].Position;
			Character newPlayer = new Character(pos, Constant.playerRadius, playerInitInfo.jobType, Constant.basicPlayerMoveSpeed);
			birthPointList[playerInitInfo.birthPointIdx].Parent = newPlayer;
			playerListLock.EnterWriteLock(); playerList.Add(newPlayer); playerListLock.ExitWriteLock();
			((Team)teamList[(int)playerInitInfo.teamID]).AddPlayer(newPlayer);
			newPlayer.TeamID = playerInitInfo.teamID;

			//设置出生点的颜色

			int cellX = Constant.GridToCellX(pos), cellY = Constant.GridToCellY(pos);
			cellColor[cellX, cellY] = TeamToColor(playerInitInfo.teamID);

			//开启装弹线程

			new Thread
				(
					() =>
					{
						while (!IsGaming) Thread.Sleep(newPlayer.CD);
						while (IsGaming)
						{
							var beginTime = Environment.TickCount64;

							var cellX = Constant.GridToCellX(newPlayer.Position);
							var cellY = Constant.GridToCellY(newPlayer.Position);
							if (cellColor[cellX, cellY] == TeamToColor(newPlayer.TeamID)) newPlayer.AddBulletNum();

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

		private bool isGaming = false;
		public bool IsGaming { get => isGaming; }

		public bool StartGame(int milliSeconds)
		{
			if (isGaming) return false;
			playerListLock.EnterReadLock();
			{
				foreach (Character player in playerList)
				{
					player.CanMove = true;
					player.AddShield(Constant.shieldTimeAtBirth);		//出生时附加盾牌
				}
			}
			playerListLock.ExitReadLock();

			//开始产生道具

			new Thread
				(
					() =>
					{
						while (!IsGaming) Thread.Sleep(1000);
						while (IsGaming)
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
				)
			{ IsBackground = true }.Start();

			isGaming = true;
			Thread.Sleep(milliSeconds);
			isGaming = false;
			playerListLock.EnterReadLock();
			{
				foreach (Character player in playerList)
				{
					player.CanMove = false;
				}
			}
			playerListLock.ExitReadLock();
			return true;
		}

		private void ProduceOneProp()
		{
			Random r = new Random((int)Environment.TickCount64);
			XYPosition newPropPos = new XYPosition();
			while (true)
			{
				newPropPos.x = r.Next(0, Rows * Constant.numOfGridPerCell);
				newPropPos.y = r.Next(0, Cols * Constant.numOfGridPerCell);
				int cellX = Constant.GridToCellX(newPropPos), cellY = Constant.GridToCellY(newPropPos);
				bool canLayProp = true;
				objListLock.EnterReadLock();
				foreach (GameObject obj in objList)
				{
					if (cellX == Constant.GridToCellX(obj.Position) && cellY == Constant.GridToCellY(obj.Position) && (obj is Wall || obj is BirthPoint))
					{
						canLayProp = false;
						break;
					}
				}
				objListLock.ExitReadLock();
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
				unpickedPropList.AddLast(newProp);
				unpickedPropListLock.ExitWriteLock();
				newProp.CanMove = true;
			}
		}

		//人物移动
		public void MovePlayer(long playerID, int moveTimeInMilliseconds, double moveDirection)
		{
			if (!isGaming) return;
			Character? playerToMove = FindPlayerFromPlayerList(playerID);
			if (playerToMove != null) MoveObj(playerToMove, moveTimeInMilliseconds, moveDirection);
		}

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
				if (((Mine)listObj).Parent.TeamID == ((Character)obj).TeamID) return false;		//同组的人不会触发地雷
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
					listLock.ExitReadLock();

					//如果越界，则与越界方块碰撞
					if (collisionObj == null && (obj.Position.x <= obj.Radius || obj.Position.y <= obj.Radius
					|| obj.Position.x >= Constant.numOfGridPerCell * Rows - obj.Radius || obj.Position.y >= Constant.numOfGridPerCell * Cols - obj.Radius))
					{
						collisionObj = new OutOfBoundBlock(nextPos);
					}
					return collisionObj;
				};

			GameObject collisionObj = null;
			if ((collisionObj = CheckCollisionInList(playerList, playerListLock)) != null || (collisionObj = CheckCollisionInList(objList, objListLock)) != null)
			{
				return collisionObj;
			}
			return null;
		}

		//碰撞后处理，返回是否已经销毁该对象
		private bool OnCollision(GameObject obj, GameObject collisionObj, Vector moveVec)
		{
			if (obj is Character)		//如果是人主动碰撞
			{

				/*由于四周是墙，所以人物永远不可能与越界方块碰撞*/

				uint maxLen = uint.MaxValue;      //移动的最大距离
				uint tmpMax;
				Vector2 objMoveUnitVector = new Vector2(1.0 * Math.Cos(obj.FacingDirection), 1.0 * Math.Sin(obj.FacingDirection));

				XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);

				//在某列表中检查碰撞
				Action<ArrayList, ReaderWriterLockSlim> FindMax =
					(ArrayList lst, ReaderWriterLockSlim listLock) =>
					{
						listLock.EnterReadLock();
						{
							foreach (GameObject listObj in lst)
							{
								//如果再走一步发生碰撞
								if (WillCollide(obj, listObj, nextPos))
								{
									switch (listObj.Shape)	//默认obj为圆形
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
						listLock.ExitReadLock();
					};

				FindMax(playerList, playerListLock);
				FindMax(objList, objListLock);

				maxLen = (uint)Math.Min(maxLen, (obj.MoveSpeed / Constant.numOfStepPerSecond));
				obj.Move(new Vector(moveVec.angle, maxLen));
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

				if (obj.IsRigid)		//CheckCollision保证了collisionObj不可能是BirthPoint
				{
					BulletBomb((Bullet)obj, collisionObj); return true;
				}

				return false;
			}
			else if (obj is Prop)
			{
				//如果越界，清除出游戏
				if (obj.Position.x <= obj.Radius || obj.Position.y <= obj.Radius
					|| obj.Position.x >= Constant.numOfGridPerCell * Rows - obj.Radius || obj.Position.y >= Constant.numOfGridPerCell * Cols - obj.Radius)
				{
					unpickedPropListLock.EnterWriteLock();
					unpickedPropList.Remove((Prop)obj);
					unpickedPropListLock.ExitWriteLock();
					return true;
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
			objListLock.EnterWriteLock();
			{
				foreach (GameObject obj in objList)
				{
					if (obj.ID == bullet.ID)
					{
						//
						objList.Remove(obj);
						break;
					}
				}
			}
			objListLock.ExitWriteLock();

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
						playerListLock.EnterWriteLock();
						{
							playerList.Remove(playerBeingShot);
						}
						playerListLock.ExitWriteLock();
						playerBeingShot.Reset();

						bullet.Parent.AddScore(Constant.addScoreWhenKillOnePlayer);  //给击杀者加分

						new Thread
							(() =>
							{

								Thread.Sleep(Constant.deadRestoreTime);

								playerBeingShot.AddShield(Constant.shieldTimeAtBirth);	//复活加个盾

								playerListLock.EnterWriteLock();
								{
									playerList.Add(playerBeingShot);
								}
								playerListLock.ExitWriteLock();

								if (IsGaming)
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
				else if (objBeingShot is Bullet)		//如果被击中的是另一个子弹，把它爆掉
				{
					new Thread(() => { BulletBomb((Bullet)objBeingShot, null); }) { IsBackground = true }.Start();
				}
			}

			/*改变地图颜色*/

			int cellX = Constant.GridToCellX(bullet.Position), cellY = Constant.GridToCellY(bullet.Position);
			var colorRange = bullet.GetColorRange();
			
			/*哪些颜色不能够被改变*/
			bool[,] cannotColor = new bool[Rows, Cols];

			objListLock.EnterReadLock();
			{
				foreach (GameObject obj in objList)
				{
					if (obj.IsRigid && (obj is Wall || obj is BirthPoint))
					{
						cannotColor[Constant.GridToCellX(obj.Position), Constant.GridToCellY(obj.Position)] = true;
					}
				}
			}
			objListLock.ExitReadLock();

			foreach (var pos in colorRange)
			{
				int colorCellX = cellX + pos.x, colorCellY = cellY + pos.y;
				if (colorCellX < 0 || colorCellX >= Rows || colorCellY < 0 || colorCellY >= Cols) continue;
				if (!cannotColor[colorCellX, colorCellY])
				{
					cellColor[colorCellX, colorCellY] = TeamToColor(bullet.Parent.TeamID);
				}
			}

			var attackRange = bullet.GetAttackRange();
			for (int i = 0; i < attackRange.GetLength(0); ++i)	//化为实际格子坐标
			{
				attackRange[i].x += cellX;
				attackRange[i].y += cellY;
			}
			ArrayList willBeAttacked = new ArrayList();
			playerListLock.EnterReadLock();
			{
				foreach (Character player in playerList)
				{
					int playerCellX = Constant.GridToCellX(player.Position), playerCellY = Constant.GridToCellY(player.Position);
					foreach (var pos in attackRange)
					{
						if (pos.x == playerCellX && pos.y == playerCellY && !object.ReferenceEquals(player, objBeingShot)) { willBeAttacked.Add(player); }
					}
				}
			}
			playerListLock.ExitReadLock();
			foreach (Character player in willBeAttacked)
			{
				BombOnePlayer(player);
			}
			willBeAttacked.Clear();

			objListLock.EnterReadLock();
			{
				foreach (Obj obj in objList)
				{
					if (obj.IsRigid && obj is Bullet)
					{
						int objCellX = Constant.GridToCellX(obj.Position), objCellY = Constant.GridToCellY(obj.Position);
						foreach (var pos in attackRange)
						{
							if (pos.x == objCellX && pos.y == objCellY && !object.ReferenceEquals(obj, objBeingShot)) { willBeAttacked.Add(obj); }
						}
					}
				}
			}
			objListLock.ExitReadLock();
			foreach (Bullet beAttakedBullet in willBeAttacked)
			{
				new Thread(() => { BulletBomb(beAttakedBullet, null); }) { IsBackground = true }.Start();
			}
			willBeAttacked.Clear();
		}

		//物体移动
		private void MoveObj(GameObject obj, int moveTime, double moveDirection)
		{
			new Thread
				(
							() =>
							{
								lock (obj.moveLock)
								{
									if (!obj.IsAvailable) return;
									obj.IsMoving = true;     //开始移动
								}

								var moveBeginTime = Environment.TickCount64;
								var moveEndTime = moveBeginTime + moveTime;
								var timeShouldBe = moveBeginTime;

								GameObject.Debug(obj, " begin to move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64.ToString());
								double deltaLen = 0.0;      //储存行走的误差
								Vector moveVec = new Vector(moveDirection, 0.0);
								//先转向
								if (isGaming && obj.CanMove) deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));     //先转向
								GameObject? collisionObj = null;
								while (isGaming && timeShouldBe < moveEndTime && obj.CanMove && !obj.IsResetting)
								{
									
									moveVec.length = obj.MoveSpeed / Constant.numOfStepPerSecond + deltaLen;
									deltaLen = 0;

									//越界情况处理：如果越界，那么一定与四周的墙碰撞，在OnCollision中检测碰撞
									//缺陷：半径为0的物体检测不到越界
									//改进：如果越界，则与越界方块碰撞
									
									while (true)
									{
										collisionObj = CheckCollision(obj, moveVec);
										if (collisionObj == null) break;
										if (collisionObj is Mine)		//CheckCollision保证只有不同组的人物会和地雷碰撞
										{
											ActivateMine((Character)obj, (Mine)collisionObj);
										}
										else
										{
											if (OnCollision(obj, collisionObj, moveVec))
											{
												//已经被销毁

												GameObject.Debug(obj, " collide with " + collisionObj.ToString() + " and has been removed from the game.");

												return;
											}
											if (obj.IsRigid && obj is Character) moveVec.length = 0;
											break;
										}
									}

									////else
									{
										deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));
									}

									timeShouldBe += 1000 / Constant.numOfStepPerSecond;
									var nowTime = Environment.TickCount64;
									
									if (timeShouldBe >= nowTime)
									{
										Thread.Sleep((int)(timeShouldBe - nowTime));
									}
									else
									{
										Console.WriteLine("The computer runs so slow that the player cannot finish moving during this time!!!!!! Time should be: {0} but nowTime is {1}!", timeShouldBe, nowTime);
									}
								}
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
								GameObject.Debug(obj, " end move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64);
								if (obj is Bullet) BulletBomb((Bullet)obj, null);
							}
						)
			{ IsBackground = true }.Start();
		}

		private Character? FindPlayerFromPlayerList(long playerID)
		{
			Character? player = null;
			playerListLock.EnterReadLock();
			{
				foreach (Character iplayer in playerList)
				{
					if (iplayer.ID == playerID)
					{
						player = iplayer;
						break;
					}
				}
			}
			playerListLock.ExitReadLock();
			return player;
		}

		//攻击
		public bool Attack(long playerID, int timeInMilliseconds, double angle)
		{
			if (!isGaming) return false;
			Character? playerWillAttack = FindPlayerFromPlayerList(playerID);

			if (playerWillAttack != null)
			{
				if (!playerWillAttack.IsAvailable) return false;
				if (playerWillAttack.Attack())
				{
					Bullet newBullet = new Bullet(
						playerWillAttack.Position + new XYPosition((int)(Constant.numOfGridPerCell * Math.Cos(angle)), (int)(Constant.numOfGridPerCell * Math.Sin(angle))),
						Constant.bulletRadius, Constant.basicBulletMoveSpeed, playerWillAttack.bulletType, playerWillAttack.AP, playerWillAttack.HasSpear);

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
					case BulletType.Bullet3:		//不断检测它所位于的格子，并将其染色
						timeInMilliseconds = int.MaxValue;
						new Thread
							(
								() =>
								{
									for (int i = 0; i < 50 && !newBullet.CanMove; ++i)		//等待子弹开始移动，最多等待50次
									{
										Thread.Sleep(1000 / Constant.numOfStepPerSecond);
									}

									while (newBullet.CanMove)
									{
										int cellX = Constant.GridToCellX(newBullet.Position), cellY = Constant.GridToCellY(newBullet.Position);
										
										if (cellX >= 0 && cellX < Rows && cellY >= 0 && cellY < Cols)
										{
											bool canColor = true;
											objListLock.EnterReadLock();
											{
												foreach (GameObject obj in objList)
												{
													if (obj.IsRigid
													&& Constant.GridToCellX(obj.Position) == cellX
													&& Constant.GridToCellY(obj.Position) == cellY
													&& (obj is Wall || obj is BirthPoint))
													{
														canColor = false;
														break;
													}
												}
											}
											objListLock.ExitReadLock();

											if (canColor)
											{
												cellColor[cellX, cellY] = TeamToColor(newBullet.Parent.TeamID);
											}
										}

										Thread.Sleep(1000 / Constant.numOfStepPerSecond);
									}
								}
							)
						{ IsBackground = true }.Start();
						
						break;
					}

					objListLock.EnterWriteLock(); objList.Add(newBullet); objListLock.ExitWriteLock();

					newBullet.CanMove = true;
					MoveObj(newBullet, timeInMilliseconds, angle);
					return true;
				}
			}

			return false;
		}

		//捡道具，是否是前面的那一格（true则是面向的那一格；false则是所在的那一格），以及要捡的道具类型
		public bool Pick(long playerID, PropType propType)
		{
			if (!IsGaming) return false;
			Character? player = FindPlayerFromPlayerList(playerID);
			if (player == null) return false;
			if (!player.IsAvailable) return false;

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
			unpickedPropListLock.ExitWriteLock();

			if (prop != null)
			{
				player.HoldProp = prop;
				prop.Parent = player;
			}

			player.IsModifyingProp = false;
			return prop != null;
		}

		public void Use(long playerID)
		{
			if (!isGaming) return;
			Character? player = FindPlayerFromPlayerList(playerID);
			if (player == null) return;

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
								objListLock.EnterWriteLock();
								objList.Add(mine);
								objListLock.ExitWriteLock();

								Thread.Sleep(Constant.mineTime);

								objListLock.EnterWriteLock();
								try { objList.Remove(mine); } catch { }
								objListLock.ExitWriteLock();
							}
						)
					{ IsBackground = true }.Start();
				}
			}
		}

		private void ActivateMine(Character player, Mine mine)
		{
			objListLock.EnterWriteLock();
			try { objList.Remove(mine); } catch { }
			objListLock.ExitWriteLock();

			switch (mine.GetPropType())
			{
			case PropType.Dirt:
				player.AddMoveSpeed(-Constant.dirtMoveSpeedDebuff, Constant.buffPropTime);
				break;
			case PropType.Attenuator:
				player.AddAP(-Constant.attenuatorAtkDebuff, Constant.buffPropTime);
				break;
			case PropType.Divider:
				player.ChangeCD(Constant.dividerCdDiscount, Constant.buffPropTime);
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
			foreach (Team team in teamList)		// team 只有在开始游戏之前被修改，开始之后是只读的，因此不须加锁
			{
				gameObjList.AddRange(team.GetPlayerListForUnsafe());
			}
			objListLock.EnterWriteLock(); gameObjList.AddRange(objList); objListLock.ExitWriteLock();
			unpickedPropListLock.EnterReadLock(); gameObjList.AddRange(unpickedPropList); unpickedPropListLock.ExitReadLock();
			return gameObjList;
		}

		public Character GetPlayerFromTeam(long playerID)	//从队伍中寻找玩家，要求一定要找到
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
			Character? player = FindPlayerFromPlayerList(playerID);
			if (player == null) return;
			if (!player.IsAvailable) return;
			Prop? oldProp = player.UseProp();
			if (oldProp == null) return;
			oldProp.ResetPosition(player.Position);
			oldProp.ResetMoveSpeed(Constant.thrownPropMoveSpeed);
			MoveObj(oldProp, moveTimeInMilliseconds, angle);
			unpickedPropListLock.EnterWriteLock();
			unpickedPropList.AddLast(oldProp);
			unpickedPropListLock.ExitWriteLock();
		}

		public int GetTeamScore(long teamID)
		{
			if (!Team.teamExists(teamID)) throw new Exception("GetTeamScore error: No this team!");
			int score = ((Team)teamList[(int)teamID]).Score;
			foreach (var color in cellColor)
			{
				if (ColorToTeam(color) == teamID) ++score;
			}
			return score;
		}
		public Map(uint[,] mapResource, int numOfTeam)
		{
			if (numOfTeam > maxTeamNum) throw new Exception("Number of teams overflows!");

			//加入队伍
			this.numOfTeam = numOfTeam;
			teamList = new ArrayList();
			for (int i = 0; i < numOfTeam; ++i)
			{
				teamList.Add(new Team());
			}

			//初始化颜色
			var rows = mapResource.GetLength(0);
			var cols = mapResource.GetLength(1);
			cellColor = new ColorType[rows, cols];
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
				{
					cellColor[i, j] = ColorType.None;
				}

			//创建列表
			objList = new ArrayList();
			playerList = new ArrayList();
			unpickedPropList = new LinkedList<Prop>();
			birthPointList = new Dictionary<uint, BirthPoint>(MapInfo.numOfBirthPoint);

			//将墙等游戏对象插入到游戏中
			for (int i = 0; i < rows; ++i)
			{
				for (int j = 0; j < cols; ++j)
				{
					switch (mapResource[i, j])
					{
					case (uint)MapInfo.MapInfoObjType.Wall:
						objListLock.EnterWriteLock(); objList.Add(new Wall(Constant.CellToGrid(i, j), Constant.wallRadius)); objListLock.ExitWriteLock();
						break;
					case (uint)MapInfo.MapInfoObjType.BirthPoint1: case (uint)MapInfo.MapInfoObjType.BirthPoint2: case (uint)MapInfo.MapInfoObjType.BirthPoint3: case (uint)MapInfo.MapInfoObjType.BirthPoint4:
					case (uint)MapInfo.MapInfoObjType.BirthPoint5: case (uint)MapInfo.MapInfoObjType.BirthPoint6: case (uint)MapInfo.MapInfoObjType.BirthPoint7: case (uint)MapInfo.MapInfoObjType.BirthPoint8:
					{
						BirthPoint newBirthPoint = new BirthPoint(Constant.CellToGrid(i, j), Constant.birthPointRadius);
						objListLock.EnterWriteLock(); objList.Add(newBirthPoint); objListLock.ExitWriteLock();
						birthPointList.Add(MapInfo.BirthPointEnumToIdx((MapInfo.MapInfoObjType)mapResource[i, j]), newBirthPoint);
						break;
					}
					}
				}
			}

		}

	}
}

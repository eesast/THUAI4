using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System;

namespace THUnity2D
{
	public class Map
	{
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
		public static XYPosition CellToGrid(int x, int y)	//求格子的中心坐标
		{
			XYPosition ret = new XYPosition(x * numOfGridPerCell + numOfGridPerCell / 2, 
				y * numOfGridPerCell + numOfGridPerCell / 2);
			return ret;
		}
		public static int GridToCellX(XYPosition pos)		//求坐标所在的格子的x坐标
		{
			return pos.x / numOfGridPerCell;
		}
		public static int GridToCellY(XYPosition pos)      //求坐标所在的格子的y坐标
		{
			return pos.y / numOfGridPerCell;
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
		public const int numOfGridPerCell = 1000;   //每个的坐标单位数
		public const int numOfStepPerSecond = 50;       //每秒行走的步数
		public const int addScoreWhenKillOnePlayer = 10;

		private const int basicPlayerMoveSpeed = numOfGridPerCell * 2;
		private const int basicBulletMoveSpeed = numOfGridPerCell * 5;


		public int ObjMaxRadius { get => numOfGridPerCell / 2; }
		public int PlayerRadius { get => ObjMaxRadius; }
		public int WallRadius { get => ObjMaxRadius; }
		public int BirthPointRadius { get => ObjMaxRadius; }
		public int BulletRadius { get => ObjMaxRadius / 2; }

		private ArrayList objList;              //游戏对象（除了玩家外）的列表
		//private object objListLock = new object();
		private ReaderWriterLockSlim objListLock = new ReaderWriterLockSlim();
		private ArrayList playerList;        //玩家列表
											 //private object playerListLock = new object();
		private ReaderWriterLockSlim playerListLock = new ReaderWriterLockSlim();
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
			Character newPlayer = new Character(pos, PlayerRadius, playerInitInfo.jobType, Map.basicPlayerMoveSpeed);
			birthPointList[playerInitInfo.birthPointIdx].Parent = newPlayer;
			playerListLock.EnterWriteLock(); playerList.Add(newPlayer); playerListLock.ExitWriteLock();
			((Team)teamList[(int)playerInitInfo.teamID]).AddPlayer(newPlayer);
			newPlayer.TeamID = playerInitInfo.teamID;

			//设置出生点的颜色

			int cellX = GridToCellX(pos), cellY = GridToCellY(pos);
			cellColor[cellX, cellY] = TeamToColor(playerInitInfo.teamID);

			//开启装弹线程

			new Thread
				(
					() =>
					{
						while (true)
						{
							var beginTime = Environment.TickCount;

							var cellX = GridToCellX(newPlayer.Position);
							var cellY = GridToCellY(newPlayer.Position);
							if (cellColor[cellX, cellY] == TeamToColor(newPlayer.TeamID)) newPlayer.AddBulletNum();

							var endTime = Environment.TickCount;
							var deltaTime = endTime - beginTime;
							if (deltaTime < newPlayer.CD)
							{
								Thread.Sleep(newPlayer.CD - deltaTime);
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
				}
			}
			playerListLock.ExitReadLock();
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

		//人物移动
		public void MovePlayer(long playerID, int moveTime, double moveDirection)
		{
			if (!isGaming) return;
			Character? playerToMove = null;
			playerListLock.EnterReadLock();
			{
				foreach (var iPlayer in playerList)
				{
					if (((Character)iPlayer).ID == playerID)
					{
						playerToMove = (Character)iPlayer;
						break;
					}
				}
			}
			playerListLock.ExitReadLock();

			if (playerToMove != null) MoveObj(playerToMove, moveTime, moveDirection);
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


			int deltaX = Math.Abs(nextPos.x - listObj.Position.x), deltaY = Math.Abs(nextPos.y - listObj.Position.y);

			//默认obj是圆形的，因为能移动的物体目前只有圆形

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

				maxLen = (uint)Math.Min(maxLen, (obj.MoveSpeed / numOfStepPerSecond));
				obj.Move(new Vector(moveVec.angle, maxLen));
				return false;
			}
			else if (obj is Bullet)
			{
				//如果越界，爆炸
				if (obj.Position.x <= obj.Radius || obj.Position.y <= obj.Radius
					|| obj.Position.x >= numOfGridPerCell * Rows - obj.Radius || obj.Position.y >= numOfGridPerCell * Cols - obj.Radius)
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
					playerBeingShot.BeAttack(bullet.AP);
					if (playerBeingShot.HP <= 0)                //如果打死了
					{
						//人被打死时会停滞1秒钟，停滞的时段内暂从列表中删除，以防止其产生任何动作（行走、攻击等）
						playerBeingShot.CanMove = false;
						playerListLock.EnterWriteLock();
						{
							playerList.Remove(playerBeingShot);
						}
						playerListLock.ExitWriteLock();
						playerBeingShot.Reset();

						bullet.Parent.AddScore(addScoreWhenKillOnePlayer);  //给击杀者加分

						new Thread
							(() =>
							{

								Thread.Sleep(1000);

								playerListLock.EnterWriteLock();
								{
									playerList.Add(playerBeingShot);
								}
								playerListLock.ExitWriteLock();
								playerBeingShot.CanMove = true;
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

			int cellX = GridToCellX(bullet.Position), cellY = GridToCellY(bullet.Position);
			var colorRange = bullet.GetColorRange();
			
			/*哪些颜色不能够被改变*/
			bool[,] cannotColor = new bool[Rows, Cols];

			objListLock.EnterReadLock();
			{
				foreach (GameObject obj in objList)
				{
					if (obj.IsRigid && (obj is Wall || obj is BirthPoint))
					{
						cannotColor[GridToCellX(obj.Position), GridToCellY(obj.Position)] = true;
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
					int playerCellX = GridToCellX(player.Position), playerCellY = GridToCellY(player.Position);
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
					if (obj is Bullet)
					{
						int objCellX = GridToCellX(obj.Position), objCellY = GridToCellY(obj.Position);
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
									if (obj.IsMoving || !obj.CanMove) return;
									obj.IsMoving = true;     //开始移动
								}

								GameObject.Debug(obj, " begin to move at " + obj.Position.ToString());
								double deltaLen = 0.0;      //储存行走的误差
								Vector moveVec = new Vector(moveDirection, 0.0);
								//先转向
								if (isGaming && obj.CanMove) deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));     //先转向
								GameObject? collisionObj = null;
								while (isGaming && moveTime > 0 && obj.CanMove)
								{
									var beginTime = Environment.TickCount;
									moveVec.length = obj.MoveSpeed / numOfStepPerSecond + deltaLen;
									deltaLen = 0;

									//越界情况处理：如果越界，那么一定与四周的墙碰撞，在OnCollision中检测碰撞
									//缺陷：半径为0的物体检测不到越界
									//未来改进方案：引入特殊的越界方块，如果越界视为与越界方块碰撞
									if ((collisionObj = CheckCollision(obj, moveVec)) != null)
									{
										if (OnCollision(obj, collisionObj, moveVec))
										{
											//已经被销毁

											GameObject.Debug(obj, " collide with " + collisionObj.ToString() + " and has been removed from the game.");

											return;
										}
										if (obj.IsRigid && obj is Character) moveVec.length = 0;
									}
									////else
									{
										deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));
									}
									var endTime = System.Environment.TickCount;
									moveTime -= 1000 / numOfStepPerSecond;
									var deltaTime = endTime - beginTime;
									if (deltaTime <= 1000 / numOfStepPerSecond)
									{
										Thread.Sleep(1000 / numOfStepPerSecond - deltaTime);
									}
									else
									{
										Console.WriteLine("The computer runs so slow that the player cannot finish moving during this time!!!!!!");
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
								GameObject.Debug(obj, " end move at " + obj.Position.ToString());
								if (obj is Bullet) BulletBomb((Bullet)obj, null);
							}
						)
			{ IsBackground = true }.Start();
		}

		//攻击
		public bool Attack(long playerID, int time, double angle)
		{
			if (!isGaming) return false;
			Character? playerWillAttack = null;
			playerListLock.EnterReadLock();
			{
				foreach (Character player in playerList)
				{
					if (player.ID == playerID)
					{
						playerWillAttack = player;
						break;
					}
				}
			}
			playerListLock.ExitReadLock();

			if (playerWillAttack != null)
			{
				if (playerWillAttack.Attack())
				{
					Bullet newBullet = new Bullet(
						playerWillAttack.Position + new XYPosition((int)(numOfGridPerCell * Math.Cos(angle)), (int)(numOfGridPerCell * Math.Sin(angle))),
						BulletRadius, basicBulletMoveSpeed, playerWillAttack.bulletType, playerWillAttack.AP);

					newBullet.Parent = playerWillAttack;
					objListLock.EnterWriteLock(); objList.Add(newBullet); objListLock.ExitWriteLock();

					newBullet.CanMove = true;
					MoveObj(newBullet, time, angle);
					return true;
				}
			}

			return false;
		}

		public ArrayList GetGameObject()
		{
			ArrayList gameObjList = new ArrayList();
			playerListLock.EnterWriteLock(); gameObjList.AddRange(playerList); playerListLock.ExitWriteLock();
			objListLock.EnterWriteLock(); gameObjList.AddRange(objList); objListLock.ExitWriteLock();
			return gameObjList;
		}

		public Character GetPlayerFromTeam(long playerID)
		{
			foreach (Team team in teamList)
			{
				Character? player = team.GetPlayer(playerID);
				if (player != null) return player;
			}
			throw new Exception("GetPlayerFromTeam error: No this player!");
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
			birthPointList = new Dictionary<uint, BirthPoint>(MapInfo.numOfBirthPoint);

			//将墙等游戏对象插入到游戏中
			for (int i = 0; i < rows; ++i)
			{
				for (int j = 0; j < cols; ++j)
				{
					switch (mapResource[i, j])
					{
					case (uint)MapInfo.MapInfoObjType.Wall:
						objListLock.EnterWriteLock(); objList.Add(new Wall(CellToGrid(i, j), WallRadius)); objListLock.ExitWriteLock();
						break;
					case (uint)MapInfo.MapInfoObjType.BirthPoint1: case (uint)MapInfo.MapInfoObjType.BirthPoint2: case (uint)MapInfo.MapInfoObjType.BirthPoint3: case (uint)MapInfo.MapInfoObjType.BirthPoint4:
					case (uint)MapInfo.MapInfoObjType.BirthPoint5: case (uint)MapInfo.MapInfoObjType.BirthPoint6: case (uint)MapInfo.MapInfoObjType.BirthPoint7: case (uint)MapInfo.MapInfoObjType.BirthPoint8:
					{
						BirthPoint newBirthPoint = new BirthPoint(CellToGrid(i, j), BirthPointRadius);
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

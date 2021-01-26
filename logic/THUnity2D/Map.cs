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

		public int ObjRadius { get => numOfGridPerCell / 2; }

		private ArrayList objList;              //游戏对象（除了玩家外）的列表
		private object objListLock = new object();
		private ArrayList playerList;        //玩家列表
		private object playerListLock = new object();
		private ArrayList teamList;             //队伍列表
		private object teamListLock = new object();
		private readonly Dictionary<uint, BirthPoint> birthPointList;	//出生点列表

		private readonly int numOfTeam;

		private readonly int basicPlayerMoveSpeed;
		private readonly int basicBulletMoveSpeed;

		public long AddPlayer(PlayerInitInfo playerInitInfo)
		{
			if (!Team.teamExists(playerInitInfo.teamID) 
				|| !MapInfo.ValidBirthPointIdx(playerInitInfo.birthPointIdx) 
				|| birthPointList[playerInitInfo.birthPointIdx].Parent != null) return GameObject.invalidID;

			XYPosition pos = birthPointList[playerInitInfo.birthPointIdx].Position;
			Character newPlayer = new Character(pos, ObjRadius, playerInitInfo.jobType, this.basicPlayerMoveSpeed);
			birthPointList[playerInitInfo.birthPointIdx].Parent = newPlayer;
			lock (playerListLock) { playerList.Add(newPlayer); }
			((Team)teamList[(int)playerInitInfo.teamID]).AddPlayer(newPlayer);
			newPlayer.TeamID = playerInitInfo.teamID;

			//设置出生点的颜色

			int cellX = GridToCellX(pos), cellY = GridToCellY(pos);
			cellColor[cellX, cellY] = TeamToColor(playerInitInfo.teamID);
			return newPlayer.ID;
		}

		private bool isGaming = false;
		public bool IsGaming { get => isGaming; }
		public bool StartGame(int milliSeconds)
		{
			if (isGaming) return false;
			lock (playerListLock)
			{
				foreach (Character player in playerList)
				{
					player.CanMove = true;
				}
			}
			isGaming = true;
			Thread.Sleep(milliSeconds);
			isGaming = false;
			lock (playerListLock)
			{
				foreach (Character player in playerList)
				{
					player.CanMove = false;
				}
			}
			return true;
		}

		//人物移动
		public void MovePlayer(long playerID, int moveTime, double moveDirection)
		{
			if (!isGaming) return;
			lock (playerListLock)
			{
				foreach (var iPlayer in playerList)
				{
					if (((Character)iPlayer).ID == playerID)
					{
						MoveObj((GameObject)iPlayer, moveTime, moveDirection);
						break;
					}
				}
			}
		}

		//碰撞检测，如果这样行走是否会与之碰撞，返回与之碰撞的物体
		private GameObject? CheckCollision(GameObject obj, Vector moveVec)
		{
			XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);

			//在某列表中检查碰撞
			Func<ArrayList, object, GameObject> CheckCollisionInList =
				(ArrayList lst, object listLock) =>
				{
					lock (listLock)
					{
						foreach (GameObject listObj in lst)
						{
							if (!listObj.IsRigid || listObj.ID == obj.ID) continue; //不检查自己和非刚体

							if (listObj is BirthPoint)			//如果是出生点，那么除了自己以外的其他玩家需要检查碰撞
							{
								//如果是角色并且出生点不是它的出生点，需要检查碰撞，否则不检查碰撞；
								//下面的条件是obj is Character && !object.ReferenceEquals(((BirthPoint)listObj).Parent, obj)求非得结果
								if (!(obj is Character) || object.ReferenceEquals(((BirthPoint)listObj).Parent, obj)) continue;
							}

							int deltaX = nextPos.x - listObj.Position.x, deltaY = nextPos.y - listObj.Position.y;
							if ((long)deltaX * deltaX + (long)deltaY * deltaY < ((long)obj.Radius + listObj.Radius) * ((long)obj.Radius + listObj.Radius))
							{
								return listObj;
							}
						}
					}
					return null;
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
				Action<ArrayList, object> FindMax =
					(ArrayList lst, object listLock) =>
					{
						lock (listLock)
						{
							foreach (GameObject listObj in lst)
							{
								if (!listObj.IsRigid || listObj.ID == obj.ID) continue; //不检查自己和非刚体

								if (listObj is BirthPoint)          //如果是出生点，那么除了自己以外的其他玩家需要检查碰撞
								{
									//如果出生点是它的出生点，不需要检查碰撞
									if (object.ReferenceEquals(((BirthPoint)listObj).Parent, obj)) continue;
								}

								int deltaX = nextPos.x - listObj.Position.x, deltaY = nextPos.y - listObj.Position.y;

								//如果再走一步发生碰撞
								if ((long)deltaX * deltaX + (long)deltaY * deltaY < ((long)obj.Radius + listObj.Radius) * ((long)obj.Radius + listObj.Radius))
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

									if (tmpMax < maxLen) maxLen = tmpMax;
								}
							}
						}
						//return null;
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
					BulletBomb((Bullet)obj, null); return true;
				}
				else if (obj.IsRigid || collisionObj is Character)	//如果碰撞对象可以被炸掉，爆炸；注意CheckCollision保证了这种情况下collisionObj不可能是BirthPoint
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
			lock (objListLock)
			{
				foreach (GameObject obj in objList)
				{
					if (obj.ID == bullet.ID)
					{
						//
						lock (objListLock) { objList.Remove(obj); }
						break;
					}
				}
			}
			
			if (objBeingShot != null && objBeingShot is Character)	//如果击中了玩家
			{
				var playerBeingShot = (Character)objBeingShot;
				if (playerBeingShot.TeamID != bullet.Parent.TeamID)		//如果击中的不是队友
				{
					playerBeingShot.BeAttack(bullet.AP);
					if (playerBeingShot.HP <= 0)				//如果打死了
					{
						//人被打死时会停滞1秒钟
						playerBeingShot.CanMove = false;
						lock (playerListLock)
						{
							playerList.Remove(playerBeingShot);
						}
						playerBeingShot.Reset();

						bullet.Parent.AddScore(addScoreWhenKillOnePlayer);  //给击杀者加分

						Thread.Sleep(1000);

						lock (playerListLock)
						{
							playerList.Add(playerBeingShot);
						}
						playerBeingShot.CanMove = true;
					}
				}
			}

			/*改变地图颜色*/

			int cellX = GridToCellX(bullet.Position), cellY = GridToCellY(bullet.Position);
			var colorRange = bullet.GetColorRange();
			
			/*哪些颜色不能够被改变*/
			bool[,] cannotColor = new bool[Rows, Cols];
			lock (objListLock)
			{
				foreach (GameObject obj in objList)
				{
					if (obj.IsRigid && (obj is Wall || obj is BirthPoint))
					{
						cannotColor[GridToCellX(obj.Position), GridToCellY(obj.Position)] = true;
					}
				}
			}

			foreach (var pos in colorRange)
			{
				int colorCellX = cellX + pos.x, colorCellY = cellY + pos.y;
				if (colorCellX < 0 || colorCellX >= Rows || colorCellY < 0 || colorCellY >= Cols) continue;
				if (!cannotColor[colorCellX, colorCellY])
				{
					cellColor[colorCellX, colorCellY] = TeamToColor(bullet.Parent.TeamID);
				}
			}
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
		public void Attack(long playerID, int time, double angle)
		{
			if (!isGaming) return;
			lock (playerListLock)
			{
				foreach (Character player in playerList)
				{
					if (player.ID == playerID)
					{
						if (player.Attack())
						{
							Bullet newBullet = new Bullet(
								player.Position + new XYPosition((int)(numOfGridPerCell * Math.Cos(angle)), (int)(numOfGridPerCell * Math.Sin(angle))),
								ObjRadius, basicBulletMoveSpeed, player.bulletType, player.AP);

							newBullet.Parent = player;
							lock (objListLock) { objList.Add(newBullet); }

							newBullet.CanMove = true;
							MoveObj(newBullet, time, angle);
						}
						break;
					}
				}
			}
		}

		public ArrayList GetGameObject()
		{
			ArrayList gameObjList = new ArrayList();
			lock (playerListLock) { gameObjList.AddRange(playerList); }
			lock (objListLock) { gameObjList.AddRange(objList); }
			return gameObjList;
		}

		public int GetPlayerScore(long playerID)
		{
			foreach (Team team in teamList)
			{
				int score;
				if (team.GetPlayerScore(playerID, out score)) return score;
			}
			throw new Exception("GetPlayerScore error: No this player!");
		}
		public int GetPlayerHP(long playerID)
		{
			foreach (Team team in teamList)
			{
				int hp;
				if (team.GetPlayerHP(playerID, out hp)) return hp;
			}
			throw new Exception("GetPlayerHP error: No this player!");
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
			teamList = ArrayList.Synchronized(new ArrayList());
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

			this.basicPlayerMoveSpeed = numOfGridPerCell * 2;       //每秒钟人物行走的坐标数
			this.basicBulletMoveSpeed = numOfGridPerCell * 5;       //每秒钟子弹行走的坐标数

			//创建线程安全的列表
			objList = ArrayList.Synchronized(new ArrayList());
			playerList = ArrayList.Synchronized(new ArrayList());
			birthPointList = new Dictionary<uint, BirthPoint>(MapInfo.numOfBirthPoint);

			//将墙等游戏对象插入到游戏中
			for (int i = 0; i < rows; ++i)
			{
				for (int j = 0; j < cols; ++j)
				{
					switch (mapResource[i, j])
					{
					case (uint)MapInfo.MapInfoObjType.Wall:
						lock (objListLock) { objList.Add(new Wall(CellToGrid(i, j), ObjRadius)); }
						break;
					case (uint)MapInfo.MapInfoObjType.BirthPoint1: case (uint)MapInfo.MapInfoObjType.BirthPoint2: case (uint)MapInfo.MapInfoObjType.BirthPoint3: case (uint)MapInfo.MapInfoObjType.BirthPoint4:
					case (uint)MapInfo.MapInfoObjType.BirthPoint5: case (uint)MapInfo.MapInfoObjType.BirthPoint6: case (uint)MapInfo.MapInfoObjType.BirthPoint7: case (uint)MapInfo.MapInfoObjType.BirthPoint8:
					{
						BirthPoint newBirthPoint = new BirthPoint(CellToGrid(i, j), ObjRadius);
						lock (objListLock) { objList.Add(newBirthPoint); }
						birthPointList.Add(MapInfo.BirthPointEnumToIdx((MapInfo.MapInfoObjType)mapResource[i, j]), newBirthPoint);
						break;
					}
					}
				}
			}

		}

	}
}

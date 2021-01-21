using System.Collections;
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
			Color4 = 4,
			Color2 = 2,
			Color3 = 3,
		}
		public struct PlayerInitInfo
		{
			public XYPosition initPos;
			public JobType jobType;
			public long teamID;
			public PlayerInitInfo(XYPosition initPos, JobType jobType, long teamID)
			{
				this.initPos = initPos;
				this.jobType = jobType;
				this.teamID = teamID;
			}
		}
		public XYPosition CellToGrid(int x, int y)	//求格子的中心坐标
		{
			XYPosition ret = new XYPosition(x * numOfGridPerCell + numOfGridPerCell / 2, 
				y * numOfGridPerCell + numOfGridPerCell / 2);
			return ret;
		}
		public int GridToCellX(XYPosition pos)		//求坐标所在的格子的x坐标
		{
			return pos.x / numOfGridPerCell;
		}
		public int GridToCellY(XYPosition pos)      //求坐标所在的格子的y坐标
		{
			return pos.y / numOfGridPerCell;
		}
		private ColorType[,] cellColor;			//储存每格的颜色
		public int Rows							//行数
		{
			get => cellColor.GetLength(0);
		}
		public int Cols							//列数
		{
			get => cellColor.GetLength(1);
		}
		public const int numOfGridPerCell = 1000;   //每个的坐标单位数
		public const int numOfStepPerSecond = 50;		//每秒行走的步数

		public int ObjRadius
		{
			get => numOfGridPerCell / 2;
		}

		private ArrayList objList;              //游戏对象（除了玩家外）的列表
		private object objListLock = new object();
		private ArrayList playerList;        //玩家列表
		private object playerListLock = new object();
		private ArrayList teamList;             //队伍列表
		private object teamListLock = new object();

		private readonly int numOfTeam;

		private readonly int basicPlayerMoveSpeed;

		public long AddPlayer(PlayerInitInfo playerInitInfo)
		{
			if (!Team.teamExists(playerInitInfo.teamID)) return GameObject.invalidID;
			Character newPlayer = new Character(playerInitInfo.initPos, ObjRadius, playerInitInfo.jobType, this.basicPlayerMoveSpeed);
			lock (playerListLock) { playerList.Add(newPlayer); }
			((Team)teamList[(int)playerInitInfo.teamID]).AddPlayer(newPlayer);
			newPlayer.TeamID = playerInitInfo.teamID;
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

		public Map(uint[,] mapResource, int numOfTeam)
		{
			//初始化颜色
			var rows = mapResource.GetLength(0);
			var cols = mapResource.GetLength(1);
			cellColor = new ColorType[rows, cols];
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
				{
					cellColor[i, j] = ColorType.None;
				}

			this.basicPlayerMoveSpeed = numOfGridPerCell * 2;		//每秒钟行走的坐标数

			//创建线程安全的列表
			objList = ArrayList.Synchronized(new ArrayList());
			playerList = ArrayList.Synchronized(new ArrayList());
			
			//将墙等游戏对象插入到游戏中
			for (int i = 0; i < rows; ++i)
			{
				for (int j = 0; j < cols; ++j)
				{
					if (mapResource[i, j] == (uint)MapInfoObjType.Wall)
					{
						lock (objListLock) { objList.Add(new Wall(CellToGrid(i, j), ObjRadius)); }
					}
				}
			}

			this.numOfTeam = numOfTeam;
			teamList = ArrayList.Synchronized(new ArrayList());
			for (int i = 0; i < numOfTeam; ++i)
			{
				teamList.Add(new Team());
			}
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
				else if (obj.IsRigid || collisionObj is Character)	//如果碰撞对象可以被炸掉，爆炸
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
					/*未完，受攻击代码*/
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
								if (!obj.CanMove) return;

								lock (obj.moveLock)
								{
									if (obj.IsMoving) return;
									obj.IsMoving = true;     //开始移动
								}

								GameObject.Debug(obj, " begin to move at " + obj.Position.ToString());
								double deltaLen = 0.0;      //储存行走的误差
								Vector moveVec = new Vector(moveDirection, 0.0);
								//先转向
								if (isGaming) deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));     //先转向
								GameObject? collisionObj = null;
								while (isGaming && moveTime > 0)
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
								ObjRadius, basicPlayerMoveSpeed, player.bulletType);

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
	}
}

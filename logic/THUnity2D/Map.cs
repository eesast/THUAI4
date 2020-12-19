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
			Color2 = 2,
			Color3 = 3,
			Color4 = 4
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

		private ArrayList objList;				//游戏对象（除了玩家外）的列表
		private ArrayList playerList;        //玩家列表

		private ArrayList teamList;				//队伍列表

		private readonly int numOfTeam;

		private readonly int basicPlayerMoveSpeed;

		public long AddPlayer(PlayerInitInfo playerInitInfo)
		{
			if (!Team.teamExists(playerInitInfo.teamID)) return GameObject.invalidID;
			Character newPlayer = new Character(playerInitInfo.initPos, ObjRadius, playerInitInfo.jobType, this.basicPlayerMoveSpeed);
			playerList.Add(newPlayer);
			((Team)teamList[(int)playerInitInfo.teamID]).AddPlayer(newPlayer);
			return newPlayer.ID;
		}

		private bool isGaming = false;
		public bool IsGaming { get => isGaming; }
		public bool StartGame(int milliSeconds)
		{
			if (isGaming) return false;
			foreach (Character player in playerList)
			{
				player.CanMove = true;
			}
			isGaming = true;
			Thread.Sleep(milliSeconds);
			isGaming = false;
			foreach (Character player in playerList)
			{
				player.CanMove = false;
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
						objList.Add(new Wall(CellToGrid(i, j), ObjRadius));
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
			foreach (var iPlayer in playerList)
			{
				if (((Character)iPlayer).ID == playerID)
				{
					new Thread
						(
							() =>
							{
								Character player = (Character)iPlayer;
								if (!player.CanMove) return;
								lock (player.gameObjLock)
								{
									if (player.IsMoving) return;
									player.IsMoving = true;		//开始移动
								}
								
								GameObject.Debug(player, " begin to move at " + player.Position.ToString());
								double deltaLen = 0.0;      //储存行走的误差
								Vector moveVec = new Vector(moveDirection, 0.0);
								//先转向
								if (isGaming) player.Move(moveVec);		//先转向
								while (isGaming && moveTime > 0)
								{
									var beginTime = Environment.TickCount;
									moveVec.length = player.MoveSpeed / numOfStepPerSecond + deltaLen;
									deltaLen = 0;
									GameObject collisionObj = null;
									if ((collisionObj = CheckCollision(player, moveVec)) != null)
									{
										OnCollision(player, collisionObj, moveVec);
										moveVec.length = 0;
									}
									else
									{
										deltaLen += moveVec.length - Math.Sqrt(player.Move(moveVec));
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
								if (CheckCollision(player, moveVec) == null)
								{
									player.Move(moveVec);
								}
								player.IsMoving = false;		//结束移动
								GameObject.Debug(player, " end move at " + player.Position.ToString());
							}
						)
					{ IsBackground = true }.Start();
					break;
				}
			}
		}

		//碰撞检测，如果这样行走是否会与之碰撞，返回与之碰撞的物体
		private GameObject? CheckCollision(GameObject obj, Vector moveVec)
		{
			XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);
			//在某列表中检查碰撞
			Func<ArrayList, GameObject> CheckCollisionInList =
				(ArrayList lst) =>
				{
					foreach(GameObject listObj in lst)
					{
						if (!listObj.IsRigid || listObj.ID == obj.ID) continue; //不检查自己和非刚体
						int deltaX = nextPos.x - listObj.Position.x, deltaY = nextPos.y - listObj.Position.y;
						if ((long)deltaX * deltaX + (long)deltaY * deltaY < ((long)obj.Radius + listObj.Radius) * ((long)obj.Radius + listObj.Radius))
						{
							return listObj;
						}
					}
					return null;
				};
			GameObject collisionObj = null;
			if ((collisionObj = CheckCollisionInList(playerList)) != null || (collisionObj = CheckCollisionInList(objList)) != null)
			{
				return collisionObj;
			}
			return null;
		}
		
		//碰撞后处理
		private void OnCollision(GameObject obj, GameObject collisionObj, Vector moveVec)
		{
			if (obj is Character)		//如果是人主动碰撞
			{
				uint maxLen = uint.MaxValue;      //移动的最大距离
				uint tmpMax;
				Vector2 objMoveUnitVector = new Vector2(1.0 * Math.Cos(obj.FacingDirection), 1.0 * Math.Sin(obj.FacingDirection));

				XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);
				//在某列表中检查碰撞
				Action<ArrayList> FindMax =
					(ArrayList lst) =>
					{
						foreach (GameObject listObj in lst)
						{
							if (!listObj.IsRigid || listObj.ID == obj.ID) continue; //不检查自己和非刚体
							int deltaX = nextPos.x - listObj.Position.x, deltaY = nextPos.y - listObj.Position.y;
							if ((long)deltaX * deltaX + (long)deltaY * deltaY < ((long)obj.Radius + listObj.Radius) * ((long)obj.Radius + listObj.Radius))
							{
								int orgDeltaX = listObj.Position.x - obj.Position.x;
								int orgDeltaY = listObj.Position.y - obj.Position.y;
								double mod = Math.Sqrt((long)orgDeltaX * orgDeltaX + (long)orgDeltaY * orgDeltaY);
								Vector2 relativePosUnitVector = new Vector2(orgDeltaX / mod, orgDeltaY / mod);
								Vector2 moveUnitVector = new Vector2(Math.Cos(moveVec.angle), Math.Sin(moveVec.angle));
								if (relativePosUnitVector * moveUnitVector <= 0) continue;
								double tmp = mod - obj.Radius - listObj.Radius;
								if (tmp <= 0)
								{
									tmpMax = 0;
								}
								else
								{
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
						//return null;
					};

				FindMax(playerList);
				FindMax(objList);

				maxLen = (uint)Math.Min(maxLen, (obj.MoveSpeed / numOfStepPerSecond));
				obj.Move(new Vector(moveVec.angle, maxLen));
			}
		}
	}
}

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
		public const int numOfGridPerCell = GameObject.unitPerCell;   //每个的坐标单位数
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
			foreach (var iPlayer in playerList)
			{
				if (((Character)iPlayer).ID == playerID)
				{
					new Thread
						(
							() =>
							{
								Character player = (Character)iPlayer;
								lock (player.gameObjLock)
								{
									if (!player.CanMove) return;
									player.CanMove = false;
								}
								
								GameObject.Debug(player, " begin to move at " + player.Position.ToString());
								double deltaLen = 0.0;      //储存行走的误差
								Vector moveVec = new Vector(moveDirection, 0.0);
								while (isGaming && moveTime > 0)
								{
									var beginTime = System.Environment.TickCount;
									moveVec.length = player.MoveSpeed / numOfStepPerSecond + deltaLen;
									deltaLen = 0;
									if (CheckCollision(player, moveVec) != null)
									{
										moveVec.length = 0;
									}
									deltaLen += moveVec.length - Math.Sqrt(player.Move(moveVec));
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
								if (CheckCollision(player, moveVec) != null)
								{
									player.Move(moveVec);
								}
								player.CanMove = true;
								GameObject.Debug(player, " end move at " + player.Position.ToString());
							}
						)
					{ IsBackground = true }.Start();
					break;
				}
			}
		}

		//碰撞检测，返回与之碰撞的物体
		private GameObject? CheckCollision(GameObject obj, Vector moveVec)
		{
			return null;		//TODO: 暂不检测碰撞
		}
	}
}

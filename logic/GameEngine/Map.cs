using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Timothy.FrameRateTask;
using THUnity2D;

namespace GameEngine
{
	
	public partial class Map
	{

		public enum ColorType
		{
			None = 0,
			Color1 = 1,
			Color2 = 2,
			Color3 = 3,
			Color4 = 4,
		}
		public static ColorType TeamToColor(long teamID)
		{
			return (ColorType)(teamID + 1L);
		}
		public static long ColorToTeam(ColorType color)
		{
			return (long)color - 1L;
		}


		private ColorType[,] cellColor;         //储存每格的颜色
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
		public void SetCellColor(int cellX, int cellY, ColorType color)
		{
			cellColor[cellX, cellY] = color;
		}
		public ColorType GetCellColor(int cellX, int cellY)
		{
			return cellColor[cellX, cellY];
		}
		public int GetColorArea(ColorType color)
		{
			int area = 0;
			foreach (var icolor in cellColor)
			{
				if (icolor == color) ++area;
			}
			return area;
		}
		public int Rows                         //行数
		{
			get => cellColor.GetLength(0);
		}
		public int Cols                         // 列数
		{
			get => cellColor.GetLength(1);
		}
		public bool OutOfBound(GameObject obj) => 
			obj.Position.x <= obj.Radius || obj.Position.y <= obj.Radius
					|| obj.Position.x >= Map.Constant.numOfGridPerCell * Rows - obj.Radius || obj.Position.y >= Constant.numOfGridPerCell * Cols - obj.Radius;

		private readonly ArrayList objList;              // 游戏对象（除了玩家外）的列表
		public ArrayList ObjList => objList;
		private readonly ReaderWriterLockSlim objListLock;       // 读写锁，防止foreach遍历出现冲突（若可改成无foreach遍历考虑去掉读写锁而用线程安全的ArrayList
		public ReaderWriterLockSlim ObjListLock => objListLock;

		private readonly ArrayList playerList;           // 玩家列表（可能要频繁通过ID查找player，但玩家最多只有8个；如果玩家更多，考虑改为SortedList
		public ArrayList PlayerList => playerList;
		private readonly ReaderWriterLockSlim playerListLock;
		public ReaderWriterLockSlim PlayerListLock => playerListLock;

		private readonly Dictionary<uint, BirthPoint> birthPointList;   // 出生点列表
		public Dictionary<uint, BirthPoint> BirthPointList => birthPointList;
		// 出生点列表暂不需要锁

		public Character? FindPlayer(long playerID)
		{
			Character? player = null;
			playerListLock.EnterReadLock();
			try
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
			finally { playerListLock.ExitReadLock(); }
			return player;
		}

		public Map(uint[,] mapResource)
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

			//创建列表
			objList = new ArrayList();
			playerList = new ArrayList();
			objListLock = new ReaderWriterLockSlim();
			playerListLock = new ReaderWriterLockSlim();

			birthPointList = new Dictionary<uint, BirthPoint>(MapInfo.numOfBirthPoint);

			//将墙等游戏对象插入到游戏中
			for (int i = 0; i < rows; ++i)
			{
				for (int j = 0; j < cols; ++j)
				{
					switch (mapResource[i, j])
					{
					case (uint)MapInfo.MapInfoObjType.Wall:
						objListLock.EnterWriteLock(); try { objList.Add(new Wall(Constant.CellToGrid(i, j), Constant.wallRadius)); } finally { objListLock.ExitWriteLock(); }
						break;
					case (uint)MapInfo.MapInfoObjType.BirthPoint1: case (uint)MapInfo.MapInfoObjType.BirthPoint2: case (uint)MapInfo.MapInfoObjType.BirthPoint3: case (uint)MapInfo.MapInfoObjType.BirthPoint4:
					case (uint)MapInfo.MapInfoObjType.BirthPoint5: case (uint)MapInfo.MapInfoObjType.BirthPoint6: case (uint)MapInfo.MapInfoObjType.BirthPoint7: case (uint)MapInfo.MapInfoObjType.BirthPoint8:
					{
						BirthPoint newBirthPoint = new BirthPoint(Constant.CellToGrid(i, j), Constant.birthPointRadius);
						objListLock.EnterWriteLock(); try { objList.Add(newBirthPoint); } finally { objListLock.ExitWriteLock(); }
						birthPointList.Add(MapInfo.BirthPointEnumToIdx((MapInfo.MapInfoObjType)mapResource[i, j]), newBirthPoint);
						break;
					}
					}
				}
			}
		}
	}
}

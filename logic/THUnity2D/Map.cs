using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
	public enum ColorType
	{
		None = 0,
		Color1 = 1,
		Color2 = 2,
		Color3 = 3,
		Color4 = 4
	}
	public class Map
	{
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
		public readonly int numOfGridPerCell;	//每个的坐标单位数

		public int ObjRadius
		{
			get => numOfGridPerCell / 2;
		}

		private ArrayList objList;				//游戏对象（除了玩家外）的列表
		private ArrayList characterList;		//玩家列表

		public Map(uint[,] mapResource, int numOfGridPerCell)
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

			this.numOfGridPerCell = numOfGridPerCell;

			//创建线程安全的列表
			objList = ArrayList.Synchronized(new ArrayList());
			characterList = ArrayList.Synchronized(new ArrayList());
			
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
		}
	}
}


/// <summary>
/// 坐标结构体XYPosition
/// </summary>

namespace THUnity2D
{
	public struct XYPosition
	{
		public int x;
		public int y;
		public XYPosition(int x, int y)
		{
			this.x = x;
			this.y = y;
		}

		public int GetDim(int dim)
		{
			return dim == 0 ? x : y;
		}

		public override string ToString()
		{
			return "(" + x.ToString() + "," + y.ToString() + ")";
		}

		public static XYPosition operator+ (XYPosition p1, XYPosition p2)
		{
			return new XYPosition(p1.x + p2.x, p1.y + p2.y);
		}
	}
}

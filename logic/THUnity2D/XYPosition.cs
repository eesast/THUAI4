
/// <summary>
/// 坐标结构体XYPosition
/// </summary>

namespace THUnity2D
{
	public struct XYPosition
	{
		public int x;
		public int y;
		public XYPosition(int x = 0, int y = 0)
		{
			this.x = x;
			this.y = y;
		}

		public override string ToString()
		{
			return "(" + x.ToString() + "," + y.ToString() + ")";
		}

		//为了提高代码复用性，减少大量X，Y对称的代码，加入此函数。
		//输入0获取x，输入其他数字获取y
		public double GetProperty(int getY)
		{
			if (getY == 0)
				return x;
			else
				return y;
		}
	}
}

using System;

/// <summary>
/// 向量结构体Vector
/// </summary>

namespace THUnity2D
{
	public struct Vector
	{
		public readonly double angle;
		public readonly double length;
		public Vector(double angle = 0.0, int length = 0)
		{
			if (length < 0)
			{
				angle += Math.PI;
				length = -length;
			}
			this.angle = Tools.CorrectAngle(angle);
			this.length = length;
		}
		public static XYPosition Vector2XY(Vector v)
        {
			return new XYPosition((int)(v.length * Math.Cos(v.angle)), (int)(v.length * Math.Sin(v.angle)));
        }
	}
}

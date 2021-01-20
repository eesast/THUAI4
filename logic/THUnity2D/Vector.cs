﻿using System;

/// <summary>
/// 向量结构体Vector
/// </summary>

namespace THUnity2D
{
	public struct Vector
	{
		public double angle;
		public double length;
		public Vector(double angle, double length)
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

	public struct Vector2
	{
		public double x;
		public double y;
		public Vector2(double x, double y)
		{
			this.x = x;
			this.y = y;
		}

		public static double operator* (Vector2 v1, Vector2 v2)
		{
			return v1.x * v2.x + v1.y * v2.y;
		}
	}
}
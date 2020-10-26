using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
    public class Vector
    {
        public readonly double angle;
        public readonly double length;
        public Vector(double angle = 0, double length = 0)
        {
            this.angle = Tools.CorrectAngle(angle);
            if (double.IsNaN(length))
                length = 0;
            this.length = length;
        }
        public static Vector operator *(Vector vector, double number)
        {
            return new Vector(vector.angle, vector.length * number);
        }
        public static Vector operator *(double number, Vector vector)
        {
            return new Vector(vector.angle, vector.length * number);
        }
        public static bool operator ==(Vector vector1, Vector vector2)
        {
            return Math.Abs(vector1.angle - vector2.angle) < 1e-8 && Math.Abs(vector1.length - vector2.length) < 1e-8;
        }
        public static bool operator !=(Vector vector1, Vector vector2)
        {
            return !(vector1 == vector2);
        }
    }
}

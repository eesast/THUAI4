using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
      public class XYPosition
    {
        public readonly double x;
        public readonly double y;
        public XYPosition(double x1 = 0, double y1 = 0) {
            x = x1;
            y = y1;
        }
        public static XYPosition operator +(XYPosition a, XYPosition b)
        {
            return new XYPosition(a.x + b.x, a.y + b.y);
        }
        public static XYPosition operator -(XYPosition a, XYPosition b)
        {
            return new XYPosition(a.x - b.x, a.y - b.y);
        }
        public static XYPosition operator *(XYPosition a, double b)
        {
            return new XYPosition(a.x * b, a.y * b);
        }
        public static XYPosition operator *(double b, XYPosition a)
        {
            return new XYPosition(a.x * b, a.y * b);
        }
        public static bool operator ==(XYPosition xy1, XYPosition xy2)
        {
            return Math.Abs(xy1.x - xy2.x) < 1e-8 && Math.Abs(xy1.y - xy2.y) < 1e-8;
        }
        public static bool operator !=(XYPosition xy1, XYPosition xy2)
        {
            return !(xy1 == xy2);
        }
        public static bool IntEqual(XYPosition xy1, XYPosition xy2)
        {
            return (int)xy1.x == (int)xy2.x && (int)xy1.y == (int)xy2.y;
        }
        public XYPosition GetMid()
        {
            return new XYPosition((int)x + 0.5, (int)y + 0.5);
        }
        public static double Distance(XYPosition position1, XYPosition position2)
        {
            return Math.Sqrt(Math.Pow(Math.Abs(position1.x - position2.x), 2) + Math.Pow(Math.Abs(position1.y - position2.y), 2));
        }
        public static double ManhattanDistance(XYPosition position1, XYPosition position2)
        {
            return Math.Abs(position1.x - position2.x) + Math.Abs(position1.y - position2.y);
        }
        public override string ToString()
        {
            return "(" + x.ToString() + "," + y.ToString() + ")";
        }

        //为了提高代码复用性，减少大量X，Y对称的代码，加入此函数。
        //输入0获取x，输入其他数字获取y
        public double GetProperty(int flag)
        {
            if (flag == 0)
                return x;
            else
                return y;
        }
    }
}

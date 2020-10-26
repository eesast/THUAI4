using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
    public class Point//从坐标转化为地图内节点
    {
        public readonly int x;
        public readonly int y;
        public Point(XYPosition loc) {
            x = (int) loc.x / 20;
            y =  (int) loc.y / 20;
        }
        public Point() {
            x = 0;
            y = 0;
        }
        public override string ToString()
        {

            return "(" + x + "," + y + ")";
        }
    }
}

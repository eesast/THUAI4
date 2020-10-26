using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
    public class Tools//常见函数
    { public static double CorrectAngle(double angle) {
            if (double.IsNaN(angle)) {
                return 0.0;
            }
            while(angle < 0)
                angle += 2 * Math.PI;
            while (angle >= 2 * Math.PI)
                angle -= 2 * Math.PI;
            return angle;
        }

    }
}

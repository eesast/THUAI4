using System;
using System.Collections.Generic;
using System.Diagnostics.SymbolStore;
using System.Dynamic;
using System.Text;
using System.Threading;

namespace THUnity2D
{
    public enum BulletType
    {   //通信组实现，对应职业
       empty = 0,
       b1 = 1,
       b2 = 2
    }
    public sealed class Bullet : Obj
    {
        private int ap;     //攻击力
        public int AP { get => ap; }
        //子弹的参数，尚未写完
        public Bullet(XYPosition initPos, int radius, int basicMoveSpeed, BulletType bulletType, int ap) 
            : base(initPos, radius, false, basicMoveSpeed, ObjType.bullet)
        {
            this.ap = ap;
            switch (bulletType)
            {
                case BulletType.empty: case BulletType.b1: 
                    IsRigid = false; 
                
                    break;
                default: 
                    IsRigid = true;
                    break;
            }
        }

        public XYPosition[] GetColorRange()
		{
            XYPosition[] range = new XYPosition[9];
            for (int i = 0; i < 3; ++i)
			{
                for (int j = 0; j < 3; ++j)
				{
                    range[i * 3 + j].x = i - 1;
                    range[i * 3 + j].y = j - 1;
				}
			}
            return range;
		}
    }
}

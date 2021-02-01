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
       Empty = 0,
       b1 = 1,
       b2 = 2
    }
    public sealed class Bullet : Obj
    {
        private int ap;     //攻击力
        public int AP { get => ap; }
        //子弹的参数，尚未写完
        public Bullet(XYPosition initPos, int radius, int basicMoveSpeed, BulletType bulletType, int ap) 
            : base(initPos, radius, false, basicMoveSpeed, ObjType.Bullet, ShapeType.Circle)
        {
            this.ap = ap;
            switch (bulletType)
            {
                case BulletType.Empty: case BulletType.b1: 
                    IsRigid = true; 
                
                    break;
                default:
                IsRigid = false;
                    break;
            }
        }

        //返回染色范围，相对自己的相对距离
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

        //返回爆炸范围，相对自己的相对距离
        public XYPosition[] GetAttackRange()
		{
            XYPosition[] range = new XYPosition[9];
            //range[0].x = range[0].y = 0;
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

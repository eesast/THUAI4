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
        //子弹的参数，尚未写完
        Bullet(XYPosition initPos, int radius, int moveSpeed, BulletType bulletType) 
            : base(initPos, radius, false, moveSpeed, ObjType.bullet)
        {
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
    }
}

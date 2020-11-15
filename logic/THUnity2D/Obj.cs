using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Security.Cryptography.X509Certificates;
using System.Text;
//using Commmunication.proto;//通信组定义结构体
//所有游戏人物，道具，子弹的基类
namespace THUnity2D
{
    public enum ObjType {
        //此枚举为调试代码，之后删除，应该用通信组的版本
        wall = 1,
        prop = 2,
        bullet = 3
    }


    public enum PropType { 
    //同上
    empty,
    p1,p2,p3,p4,p5,p6
    }


    public class Obj: GameObject //道具，墙
    {
        public ObjType objType;//通信组实现
      
        public PropType _prop;//道具种类，或人物持有的道具类型
        public PropType Prop {
            set { _prop = value; }
            get { return _prop; }
        }
        public BulletType _bullet;
        public BulletType Bullet {
            set { _bullet = value; }
            get { return _bullet; }        
        }

        public Obj(double x_t, double y_t, ObjType objType) : base(new THUnity2D.XYPosition(x_t, y_t))//类型转换
        {
            this.objType = objType;
        }




        public override string ToString()
        {
            return objType + ":" + ID + ", " + Position.ToString() + " ";
        }


    }
}

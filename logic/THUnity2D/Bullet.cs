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
       b0 ,b1, b2, b3, b4, b5, b6
    }
    public class Bullet : GameObject//对象建立便开始移动，移动过程中进行碰撞检测, todo，未完成子弹在地图中的删除
    {   protected System.Threading.Timer MaxRangeTimer;
        protected System.Threading.Thread BlockWatcher;
        static protected int cur_id=0;
        protected int bullet_id;
        protected BulletType bullettype;
        protected Vector target;//目标点或者方向
        protected Vector Target {
            get => target;
            set {//正常这个是不能改的，也许反弹？？？未实现

                target = value;
            }
        }
        public BulletType Bullettype {
            get => bullettype;
        }
        protected double exploderange;
        public double Exploderange {
            get => exploderange;
        }
        double movespeed;
        public double  Movespeed{
            get=>movespeed;
        }
        public readonly bool can_crash;
        public readonly int teamid;
        protected double maxrange;//子弹到射程后析构,考虑再建立一个事件队列
        public double Maxrange {
            get => Maxrange;
        }
        protected double movedis;//当前走过的路程
        protected double Movedis {
            get => movedis;
            set {
                if (movedis <= maxrange)
                    movedis = value;
                else
                    movedis = maxrange;
            }
        }

        protected int damage;
        protected int Damage {
            get => damage;
        }

        public virtual void hitted() {// 子弹击中后，染色+造成伤害+从地图移除(这个得把人和子弹共享到一张图上。。。)
        
        
        
        }

        public Bullet(BulletType bt,double x, double y, Vector tag):base(new XYPosition(x,y)) {
            this.Movable = true;
            target = tag;
            bullet_id = cur_id;
            cur_id++;
            can_crash = true;
            BlockWatcher = new System.Threading.Thread(()=> {
                if (bt != BulletType.b1)
                    while (true)
                    {
                        if (Mapinfo.map[point.x, point.y] == 5)
                        {
                            this.Velocity = new Vector(0, 0);
                            Debug(this, "bullet hit block,boom");
                            hitted();
                            MaxRangeTimer.Change(-1, 0);
                            break;
                        }
                    }
                else
                { }
            });
            MaxRangeTimer = new System.Threading.Timer((i) => { 
            this.Velocity = new Vector(0, 0);
                Debug(this, "bullet arrive max range,boom");
                hitted();
            }
             );
            switch (bt) {
                case BulletType.b0:
                    damage = 30;
                    maxrange = 15;//格数
                    movespeed = 200;//
                    exploderange = 0;
                    break;
                case BulletType.b1://投掷
                    can_crash = false;
                    damage = 35;
                    maxrange = 12;
                    exploderange = 1;//以中心为界，半径
                    movespeed = 170;
                    break;
                case BulletType.b2:
                    damage = 40;
                    maxrange = 14;
                    exploderange = 1;
                    movespeed = 170;
                    break;
                case BulletType.b3://注意路途中染色,重写move函数
                    damage = 10;
                    maxrange = 10;
                    movespeed = 200;
                    exploderange = 0;
                    break;
                case BulletType.b4:
                    damage = 60;
                    maxrange = 15;
                    exploderange = 0;
                    movespeed = 300;
                    break;
                case BulletType.b5:
                    damage = 10;
                    maxrange = 12;
                    movespeed = 200;
                    exploderange = -1;//不能染色
                    break;
                case BulletType.b6:
                    damage = 60;
                    maxrange = 1;
                    movespeed = 500;
                    exploderange = 1;
                    break;
            }
            Velocity = new Vector(target.angle, movespeed);
            BlockWatcher.Start();
            if (bt == BulletType.b1)
            {
                MaxRangeTimer.Change((int)(target.length / movespeed)*1000, 0);
            }
            else {
                MaxRangeTimer.Change((int)(maxrange*20/movespeed)*1000,0);
            }
        }

    }
}

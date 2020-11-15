using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{    //人物的基类,暂时没有设置通信功能
    public enum JobType
    {     //同上
        job0, job1, job2, job3, job4, job5, job6, job7
    }
    public class Character : GameObject
    {   protected System.Threading.Timer MoveStopTimer;
        
        
        public readonly int teamid;
        protected double movespeed;
        protected double extraspeed=0;
        protected double Movespeed {
            get => movespeed;
            set {
                movespeed = value;//调试
            }
        }

        protected JobType job;//职业类型
        public JobType Job
        {
            set { job = value; }
            get { return job; }
        }
        protected double cd;//人物子弹发射cd，计时器实现，（todo）
        protected int max_bullet_num;
        public int Max_bullet_num {
            get { return max_bullet_num; }
        }

        protected int bullet_num;
        public int Bullet_num {
            get { return bullet_num; }
            set {
                if (value < 0)
                    bullet_num = 0;
                else if (value > max_bullet_num)
                    bullet_num = max_bullet_num;
                else
                    bullet_num = value;           
            }
        }
        protected int max_hp;
        protected int Max_hp {
            get { return max_hp; }       
        }
        protected int hp;
        protected int Hp {
            get { return hp; }
            set {
                if (value < 0)
                    hp = 0;
                else if (value <= Max_hp)
                    hp = value;
                else
                    hp = Max_hp;
            }
        
        }

        public double Cd {
            get => cd;
            set {
                cd = value;
            }
        }

        protected PropType proptype;//人物持有的道具类型。

        protected BulletType bullettype;//人物的发射子弹类型，射程伤害等信息存在子弹里

        public virtual void move(int ms, double angle) { //
            if (ms > 0)
            {
                base.Velocity = new Vector(angle, 50*movespeed);//移动即开始
                MoveStopTimer.Change(ms,0);
                
            }
        }
        public virtual void attack(Vector loc) {//角度和方向 
                                                //新建一个bullet对象,把当前的位置信息传进去
            Debug(this,"attacked");
            if (Bullet_num >= 0)
                Bullet_num -= 1;
            else
            {
                Debug(this,"no bullet left");
                return;
            }
            new Bullet(bullettype,_position.x,_position.y,loc);


        }

        /*
    1. 增加一段时间的移动速度
    2. 增加一段时间的子弹对敌人的伤害（以子弹从手中发射的时间计）
    3. 一段时间内装弹CD减短
    4. 回复大量的生命值
    5. 盾：使用后自己在一段时间内免受伤害，且打到自己的子弹不会有任何染色效果。
    6. 复活甲：使用后一段时间内消失，如果在消失之前如果被击中死亡，则原地满血满子弹复活，就像没死过一样。  
  + debuff类道具
    1. debuff地雷：在自己的脚下放地雷，一段时间后消失。地雷被埋下后就是不可见的，如果在消失前有人踩到了这一格，则会受到debuff，地雷造成伤害后立即消失，且地雷不会伤害到队友。地雷的debuff就是把上面buff中1~4的效果反过来。
    2. 眩晕地雷：同debuff地雷，但踩到后不是debuff，而是一段时间内不能行动，阻塞在原地。
  + 其它类道具
    1. 破盾：使用后一段时间内发射出的子弹无视盾的效果（时间以发射子弹的时刻计）
         */
        public virtual void useprop(Vector loc) { //角度方向,大部分道具需要添加计时器线程，参考bullet.cs的实现，具体没写完，我会尽快完成
            if (proptype == PropType.empty)
                return;
            switch (proptype)
            {
                case PropType.p1:
                    extraspeed = 100;
                    break;
                case PropType.p2:

                    break;
                case PropType.p3:
                    break;
                case PropType.p4:
                    hp = Max_hp;
                    break;
                case PropType.p5:
                    break;
                case PropType.p6:
                    break;
                    //todo

            }
            proptype = PropType.empty;
            return;

        }

        public virtual void _throw(Vector loc){

        }

        public virtual void addbullet(int num) {//上子弹 
        

        }
        /*
  1. 职业1：投掷型武器，子弹飞行的过程中不受任何干扰，直接命中目标点，伤害与染色范围（3*3）一般，子弹飞行速度快，装弹CD一般，生命值一般，人物移动速度一般。ok  
  2. 职业2：导弹型武器，可以向某一方向发射，遇到障碍或到达地图边界或达到射程上限时爆炸，伤害大，染色范围大，子弹飞行速度慢，装弹CD长，生命值较高，人物移动速度较慢。
  3. 职业3：拖泥带水型武器，子弹沿某一方向发射，会把子弹经过的路线全部染色，遇到障碍时或到达地图边界时爆炸，造成的伤害较小，爆炸处染色范围大，子弹飞行速度慢，装弹CD长，生命值较高，人物移动速度较快。  
  4. 职业4：攻击型武器，子弹飞行不受任何干扰，直接命中目标点，子弹飞行速度极快，对敌人伤害极大，但染色范围极小（只有命中的一格），装弹CD长，生命值较低，人物移动速度极快。  
  5. 职业5：人物拖泥带水：走路时会沿途把自己脚下的地图块染色，子弹像某一方向发射，遇到障碍或到达地图边界时爆炸，爆炸处伤害极小，子弹不能染色，生命值很低，装弹CD长，跑的快。  
  6. 职业6：近战型武器。子弹使用后立即爆炸，爆炸时会把人物前面的一定范围内的全部染色，伤害极高，装弹CD长，生命值较低，人物移动速度快。
  7. 职业7：反弹伤害。与无职业子弹类型相同，子弹速度一般，生命值较高，移动速度慢。（子弹有显示主人的属性）
         */

         public  Character(JobType jobtype,double x,double y): base(new XYPosition(x, y)){
            job = jobtype;
            this.Movable = true;
            MoveStopTimer = new System.Threading.Timer((i)=>
            {
                Velocity = new Vector(this.Velocity.angle, 0);
                Debug(this, "now" + this.point.ToString());
                Debug(this,"stop successfully");
            }
            ); 
            switch (job) {//按照职业类型初始化，一些基础数据
                case JobType.job0://无职业，各数据的参考值,子弹的对应关系待实现, 速度是个相对值
                    max_hp = 100;
                    max_bullet_num = 3;
                    cd = 1.5;
                    movespeed = 3;//相对值，就是标一下哪个快 
                    bullettype = BulletType.b0;
                    break;
                case JobType.job1:
                    max_hp = 100;
                    max_bullet_num = 3;
                    cd = 1.5;
                    movespeed = 3;
                    bullettype = BulletType.b1;
                    break;
                case JobType.job2:
                    max_hp = 120;
                    max_bullet_num = 3;
                    cd = 2.5;
                    movespeed = 2.5;
                    bullettype = BulletType.b2;
                    break;
                case JobType.job3:
                    max_hp = 120;
                    max_bullet_num = 2; 
                    cd = 3;
                    movespeed = 3.5;
                    bullettype = BulletType.b3;
                    break;
                case JobType.job4:
                    max_hp = 70;
                    max_bullet_num = 3;
                    cd = 3;
                    movespeed = 3.5;
                    bullettype = BulletType.b4;
                    break;
                case JobType.job5:
                    max_hp = 70;
                    max_bullet_num = 3;
                    cd = 2.5;
                    movespeed = 4;
                    bullettype = BulletType.b5;
                    break;
                case JobType.job6:
                    max_hp = 70;
                    max_bullet_num = 3;
                    cd = 2;
                    movespeed = 3.5;
                    bullettype = BulletType.b6;
                    break;
                case JobType.job7:
                    max_hp = 130;
                    max_bullet_num = 3;
                    cd = 1.5;
                    movespeed = 1.5;
                    bullettype = BulletType.b0;
                    break;
            }
            hp = max_hp;
            bullet_num = max_bullet_num;
        }
    }
}

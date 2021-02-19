using System;
using System.Windows.Forms;

namespace Logic.Client
{
    static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            for (int i = 0; i < 50; i++)
            {
                for (int j = 0; j < 50; j++)
                {
                    System.Random random = new System.Random();
                    ColorState[i, j] = random.Next(-2, 5);
                }
            }
            Application.Run(new Form1());
        }
        static int[,] ColorState = new int[50, 50];  //储存每个地图格的染色状态 0:未被染色 i:第i队染色 -1:墙体 -2:出生点
        public static int GetColorState(int x, int y)  //类外取颜色信息
        {
            return ColorState[x, y];
        }

    }
    class Player  //玩家显示类
    {
        public int x; //精细坐标
        public int y; //精细坐标
        public byte teamnum;
        public byte playernum;
        public enum Job //职业
        { Profession0 = 0, Profession1 = 1, Profession2 = 2, Profession3 = 3, Profession4 = 4, Profession5 = 5, Profession6 = 6 }
        public enum Possession //持有的道具种类
        {
            None = 0,
            SpeedUp = 1,
            DamageUp = 2,
            Shorter_Reload_CD = 3,

        };
        public Job job;
        public Possession possession = 0;
        public short health = 0;
        public byte shield = 0;  //0无状态 1有盾 2有复活甲(revive)
        public byte speed = 0;  //0无状态 1增益 2负面效果
        public byte damage = 0;  //0无状态 1增益 2负面效果
        public byte stunned = 0;  //0无状态 1被眩晕
        public bool existed = false;
        public Player(int x, int y, byte teamnum, byte playernum, Job job, short health)
        {
            this.x = x;
            this.y = y;
            this.teamnum = teamnum;
            this.playernum = playernum;
            this.health = health;
            this.job = job;
        }
    }
    class Bullet  //子弹类
    {
        public int x; //横坐标(精细)
        public int y; //纵坐标(精细)
        public int teamnum;
        public bool towards;//0:横向  1:纵向
        public bool existed = false;
        public long id = 0;
        public static long ID = 0;
        public Bullet(int x, int y, int teamnum, bool towards)
        {
            this.x = x;
            this.y = y;
            this.teamnum = teamnum;
            this.towards = towards;
            this.id = ID;
            ID++;
        }
    }
    class Item  //道具类
    {
        public int xnum; //横坐标大格
        public int ynum; //纵坐标大格
        public byte type = 0;
        /*
         * 0:加速
         * 1:增伤
         * 2:减CD
         * 3:回血
         * 4:盾
         * 5:复活甲
         * 6:减速地雷
         * 7:减伤地雷
         * 8:加CD地雷
         * 9:扣血地雷
         * 10:眩晕地雷
         * 11:破盾
         */
        public bool existed = false;
        public long id = 0;
        public static long ID = 0;
        public Item(int xnum, int ynum, byte type)
        {
            this.xnum = xnum;
            this.ynum = ynum;
            this.type = type;
            this.id = ID;
            ID++;
        }
    }

}

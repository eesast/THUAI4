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
        static int[,] ColorState = new int[50, 50];  //����ÿ����ͼ���Ⱦɫ״̬ 0:δ��Ⱦɫ i:��i��Ⱦɫ -1:ǽ�� -2:������
        public static int GetColorState(int x, int y)  //����ȡ��ɫ��Ϣ
        {
            return ColorState[x, y];
        }

    }
    class Player  //�����ʾ��
    {
        public int x; //��ϸ����
        public int y; //��ϸ����
        public byte teamnum;
        public byte playernum;
        public enum Job //ְҵ
        { Profession0 = 0, Profession1 = 1, Profession2 = 2, Profession3 = 3, Profession4 = 4, Profession5 = 5, Profession6 = 6 }
        public enum Possession //���еĵ�������
        {
            None = 0,
            SpeedUp = 1,
            DamageUp = 2,
            Shorter_Reload_CD = 3,

        };
        public Job job;
        public Possession possession = 0;
        public short health = 0;
        public byte shield = 0;  //0��״̬ 1�ж� 2�и����(revive)
        public byte speed = 0;  //0��״̬ 1���� 2����Ч��
        public byte damage = 0;  //0��״̬ 1���� 2����Ч��
        public byte stunned = 0;  //0��״̬ 1��ѣ��
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
    class Bullet  //�ӵ���
    {
        public int x; //������(��ϸ)
        public int y; //������(��ϸ)
        public int teamnum;
        public bool towards;//0:����  1:����
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
    class Item  //������
    {
        public int xnum; //��������
        public int ynum; //��������
        public byte type = 0;
        /*
         * 0:����
         * 1:����
         * 2:��CD
         * 3:��Ѫ
         * 4:��
         * 5:�����
         * 6:���ٵ���
         * 7:���˵���
         * 8:��CD����
         * 9:��Ѫ����
         * 10:ѣ�ε���
         * 11:�ƶ�
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

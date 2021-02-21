using System;
using System.Collections.Generic;
using System.Windows.Forms;
using CommandLine;
using Communication.Proto;

namespace Logic.Client
{
    static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            //ȡ�ÿ�ʼ����
            AOption? options = null;
            Parser.Default.ParseArguments<AOption>(args).WithParsed(o => { options = o; });
            if (options == null)
            {
                Application.Run(new Starting());
            }
            else
            {
                teamID = options.teamID;
                playerID = options.playerID;
                jobType = (JobType)options.job;
            }
            //��server����Ϣ
            MessageToServer msg01 = new MessageToServer();
            msg01.MessageType = MessageType.AddPlayer;
            msg01.TeamID = teamID-1;
            msg01.PlayerID = playerID-1;
            msg01.JobType = jobType;
            //TO DO:������Ϣ

            //�ж��Ƿ�ɹ��������յ���Ϣ��msg02��
            MessageToOneClient msg02 = new MessageToOneClient();
            if (!OnReciveShort(msg02)) Application.Exit();//��ʧ�����������
            else MessageBox.Show("Loading");//�ȴ���Ϸ��ʼ

            //TO DO:һ�յ���Ϣ����OnreciveNormal����

        }
        public static int[,] ColorState = new int[50, 50];  //����ÿ����ͼ���Ⱦɫ״̬ 0:δ��Ⱦɫ i:��i��Ⱦɫ -1:ǽ�� -2:������
        public static Int64 teamID;
        public static Int64 playerID;
        public static JobType jobType;
        public static int x; //��ϸ����
        public static int y; //��ϸ����
        public static int movespeed;
        private static Dictionary<Int64, Tuple<int, int>> Hashable;
        private static Form1 gameform; //��Ϸ����
        private static bool OnReciveShort(MessageToOneClient msg)  //�����Ƿ�ɹ�
        {
            if (msg.MessageType == MessageType.ValidPlayer) return true;
            else return false;
        }
        private static void OnReciveNormal(MessageToClient msg) //������Ϸ��Ϣ
        {
            switch (msg.MessageType)
            {
                case MessageType.StartGame:
                    Hashable = new Dictionary<Int64, Tuple<int, int>>();
                    for(int i = 0; i < 4; i++)
                    {
                        for(int j = 0; j < 4; j++)
                        {
                            Hashable.Add(msg.PlayerGUIDs[i].TeammateGUIDs[j], new Tuple<int, int>(i, j));
                        }
                    }
                    movespeed = msg.SelfInfo.MoveSpeed;
                    gameform = new Form1();
                    Application.Run(gameform);
                    break;
                case MessageType.Gaming:
                    Refresh(msg);
                    break;
                case MessageType.EndGame:
                    gameform.Close();
                    Application.Exit();
                    break;
                default:break;
            }
        }
        private static void Refresh(MessageToClient msg)
        {
            x = msg.SelfInfo.X;
            y = msg.SelfInfo.Y;
            movespeed = msg.SelfInfo.MoveSpeed;
            for (int i = 0; i < 50; i++) //��ȡ��ɫ
            {
                for (int j = 0; j < 50; j++)
                {
                    ColorState[i, j] = (int)msg.CellColors[j].RowColors[i];
                }
            }
            foreach(var objinfo in msg.GameObjs)
            {
                Objdeal(objinfo);
            }
            gameform.Rebuild();
        }
        private static void Objdeal(GameObjInfo obj)
        {
            switch (obj.GameObjType)
            {
                case GameObjType.BirthPoint:
                    ColorState[obj.Y / 1024, obj.X / 1024] = -2;
                    break;
                case GameObjType.Wall:
                    ColorState[obj.Y / 1024, obj.X / 1024] = -1;
                    break;
                case GameObjType.Character:
                    if (obj.IsDying) break;
                    Player player = new Player(obj.Y, obj.X, (byte)(obj.TeamID+1), (byte)(Hashable[obj.Guid].Item2+1), obj.JobType, (short)obj.Hp);
                    gameform.DrawPlayer(player);
                    break;
                case GameObjType.Bullet:
                    Bullet bullet = new Bullet(obj.Y, obj.X, (int)obj.TeamID+1, obj.BulletType, obj.Guid);
                    gameform.DrawBullet(bullet);
                    break;
                case GameObjType.Prop:
                    Item item = new Item(obj.Y / 1024, obj.X / 1024, obj.PropType);
                    gameform.DrawItem(item);
                    break;
            }
        }
    }
    public class Player  //�����ʾ��
    {
        public int x; //��ϸ����
        public int y; //��ϸ����
        public byte teamnum;
        public byte playernum;
        public JobType job;
        public PropType possession = 0;
        public short health = 0;
        public bool existed = false;
        public Player(int x, int y, byte teamnum, byte playernum, JobType job, short health)
        {
            this.x = x;
            this.y = y;
            this.teamnum = teamnum;
            this.playernum = playernum;
            this.health = health;
            this.job = job;
        }
    }
    public class Bullet  //�ӵ���
    {
        public int x; //������(��ϸ)
        public int y; //������(��ϸ)
        public Int64 teamnum;
        public BulletType bulletType;
        public bool existed = false;
        public Int64 id = 0;
        public Bullet(int x, int y, int teamnum, BulletType bulletType,Int64 guid)
        {
            this.x = x;
            this.y = y;
            this.teamnum = teamnum;
            this.bulletType = bulletType;
            this.id = guid;
        }
    }
    public class Item  //������
    {
        public int xnum; //��������
        public int ynum; //��������
        public byte type = 0;
        /*
         * 0:NULL
         * 1:����
         * 2:����
         * 4:��Ѫ
         * 5:��
         * 6:�����
         * 7:�ƶ�
         * 8:���ٵ���
         * 9:���˵���
         * 10:��CD����
         */
        public bool existed = false;
        public long id = 0;
        public Item(int xnum, int ynum, PropType type)
        {
            this.xnum = xnum;
            this.ynum = ynum;
            this.type = (byte)type;
            this.id = xnum*50+ynum;
        }
    }

}

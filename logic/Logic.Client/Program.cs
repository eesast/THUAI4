using System;
using System.Collections.Generic;
using System.Windows.Forms;
using CommandLine;
using Communication.Proto;
using Communication.CSharpClient;

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
            //取得开始命令
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
                port = options.ServerPort;
            }
            clientCommunicator = new Communication.CSharpClient.CSharpClient();
            if (clientCommunicator.Connect("127.0.0.1", (ushort)port))
            {
                MessageBox.Show("成功连接Agent");
            }
            else
            {
                MessageBox.Show("连接Agent失败");
                return;
            }
            //向server发消息
            MessageToServer msg01 = new MessageToServer();
            msg01.MessageType = MessageType.AddPlayer;
            msg01.TeamID = teamID - 1;
            msg01.PlayerID = playerID - 1;
            msg01.JobType = jobType;
            //TO DO:发出消息
            clientCommunicator.OnReceive += delegate ()
            {
                if (clientCommunicator.TryTake(out IMsg msg))
                {
                    if (msg.PacketType == PacketType.MessageToOneClient)
                    {
                        MessageToOneClient mm = msg.Content as MessageToOneClient;
                        OnReciveShort(mm);
                    }
                    else if (msg.PacketType == PacketType.MessageToClient)
                    {
                        MessageToClient mm = msg.Content as MessageToClient;
                        OnReciveNormal(mm);
                    }
                }
            };
            clientCommunicator.SendMessage(msg01);
        }
        public static int[,] ColorState = new int[50, 50];  //储存每个地图格的染色状态 0:未被染色 i:第i队染色 -1:墙体 -2:出生点
        public static Int64 teamID;
        public static Int64 playerID;
        public static JobType jobType;
        public static int port;
        public static int x; //精细坐标
        public static int y; //精细坐标
        public static int movespeed;
        private static Dictionary<Int64, Tuple<int, int>> Hashable;
        private static Form1 gameform; //游戏窗体
        public static CSharpClient clientCommunicator;
        private static void OnReciveShort(MessageToOneClient msg)  //连接是否成功
        {
            if (msg.MessageType == MessageType.ValidPlayer) { MessageBox.Show("Loading");gameform = new Form1();Application.Run(gameform); }
            else Application.Exit();
        }
        private static void OnReciveNormal(MessageToClient msg) //处理游戏消息
        {
            switch (msg.MessageType)
            {
                case MessageType.StartGame:
                    Hashable = new Dictionary<Int64, Tuple<int, int>>();
                    for (int i = 0; i < 4; i++)
                    {
                        for (int j = 0; j < 4; j++)
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
                default: break;
            }
        }
        private static void Refresh(MessageToClient msg)
        {
            x = msg.SelfInfo.X;
            y = msg.SelfInfo.Y;
            movespeed = msg.SelfInfo.MoveSpeed;
            for (int i = 0; i < 50; i++) //读取颜色
            {
                for (int j = 0; j < 50; j++)
                {
                    ColorState[i, j] = (int)msg.CellColors[j].RowColors[i];
                }
            }
            foreach (var objinfo in msg.GameObjs)
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
                    Player player = new Player(obj.Y, obj.X, (byte)(obj.TeamID + 1), (byte)(Hashable[obj.Guid].Item2 + 1), obj.JobType, (short)obj.Hp);
                    gameform.DrawPlayer(player);
                    break;
                case GameObjType.Bullet:
                    Bullet bullet = new Bullet(obj.Y, obj.X, (int)obj.TeamID + 1, obj.BulletType, obj.Guid);
                    gameform.DrawBullet(bullet);
                    break;
                case GameObjType.Prop:
                    Item item = new Item(obj.Y / 1024, obj.X / 1024, obj.PropType);
                    gameform.DrawItem(item);
                    break;
            }
        }
    }
    public class Player  //玩家显示类
    {
        public int x; //精细坐标
        public int y; //精细坐标
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
    public class Bullet  //子弹类
    {
        public int x; //横坐标(精细)
        public int y; //纵坐标(精细)
        public Int64 teamnum;
        public BulletType bulletType;
        public bool existed = false;
        public Int64 id = 0;
        public Bullet(int x, int y, int teamnum, BulletType bulletType, Int64 guid)
        {
            this.x = x;
            this.y = y;
            this.teamnum = teamnum;
            this.bulletType = bulletType;
            this.id = guid;
        }
    }
    public class Item  //道具类
    {
        public int xnum; //横坐标大格
        public int ynum; //纵坐标大格
        public byte type = 0;
        /*
         * 0:NULL
         * 1:加速
         * 2:加伤
         * 4:回血
         * 5:盾
         * 6:复活甲
         * 7:破盾
         * 8:减速地雷
         * 9:减伤地雷
         * 10:加CD地雷
         */
        public bool existed = false;
        public long id = 0;
        public Item(int xnum, int ynum, PropType type)
        {
            this.xnum = xnum;
            this.ynum = ynum;
            this.type = (byte)type;
            this.id = xnum * 50 + ynum;
        }
    }
}
using System;
using System.Collections.Generic;
using System.Windows.Forms;
using CommandLine;
using Communication.Proto;
using Communication.CSharpClient;
using System.Threading;
using System.Collections.Concurrent;

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
            Int64 teamid, playerid;
            ClientData client;
            if (options == null)
            {
                Starting starting = new Starting();
                Application.Run(starting);
                teamid = starting.teamid;
                playerid = starting.playerid;
                client = new ClientData(starting.teamid, starting.playerid, starting.job, starting.port);
            }
            else
            {
                teamid = options.teamID;
                playerid = options.playerID;
                client = new ClientData(options.teamID, options.playerID, (JobType)options.job, options.ServerPort);
            }
            if (clientCommunicator.Connect("127.0.0.1", client.port))
            {
            }
            else
            {
                MessageBox.Show("连接Agent失败");
                return;
            }
            //向server发消息
            MessageToServer msg = new MessageToServer();
            msg.MessageType = MessageType.AddPlayer;
            msg.TeamID = teamid;
            msg.PlayerID = playerid;
            msg.JobType = client.jobType;
            //TO DO:发出消息
            clientCommunicator.OnReceive += delegate ()
            {
                if (clientCommunicator.TryTake(out IMsg msg))
                    client.messageline.Add(new Tuple<IMsg, long>(msg, System.Environment.TickCount64));
            };
            client.messageline = new BlockingCollection<Tuple<IMsg, long>>();
            int bulletspeed;
            switch (client.jobType)
            {
                default:
                case JobType.Job0:
                    bulletspeed = Program.basicBulletMoveSpeed;
                    break;
                case JobType.Job1:
                    bulletspeed = Program.basicBulletMoveSpeed * 2;
                    break;
                case JobType.Job2:
                    bulletspeed = Program.basicBulletMoveSpeed / 2;
                    break;
                case JobType.Job3:
                    bulletspeed = Program.basicBulletMoveSpeed / 2;
                    break;
                case JobType.Job4:
                    bulletspeed = Program.basicBulletMoveSpeed * 4;
                    break;
                case JobType.Job5:
                    bulletspeed = Program.basicBulletMoveSpeed;
                    break;
                case JobType.Job6:
                    bulletspeed = Program.basicBulletMoveSpeed;
                    break;
            }
            client.gameform = new Form1(teamid, playerid, bulletspeed, client.jobType);
            new Thread(() =>
            {
                while (true)
                {
                    if (client.messageline.TryTake(out Tuple<IMsg, long> msg))
                    {
                        if (msg.Item1.PacketType == PacketType.MessageToOneClient)
                        {
                            MessageToOneClient mm = msg.Item1.Content as MessageToOneClient;
                            if (mm.TeamID == teamid && mm.PlayerID == playerid) client.OnReciveShort(mm);
                        }
                        else if (msg.Item1.PacketType == PacketType.MessageToClient)
                        {
                            MessageToClient mm = msg.Item1.Content as MessageToClient;
                            if (mm.TeamID == teamid && mm.PlayerID == playerid) client.OnReciveNormal(mm, msg.Item2);
                        }
                    }
                }
            }
                ).Start();
            clientCommunicator.SendMessage(msg);
            Application.Run(client.gameform);
            Application.Exit();
        }
        public const int cell = 1000;
        public const int basicBulletMoveSpeed = cell * 6;
        public static bool initneed = true;
        public static CSharpClient clientCommunicator = new CSharpClient();
    }
    public class Player  //玩家显示类
    {
        public int x; //精细坐标
        public int y; //精细坐标
        public byte teamnum;
        public byte playernum;
        public Int64 id;
        public JobType job;
        public PropType possession = 0;
        public short health = 0;
        public Player(int x, int y, byte teamnum, byte playernum, JobType job, short health, Int64 guid, PropType possession)
        {
            this.x = x;
            this.y = y;
            this.teamnum = teamnum;
            this.playernum = playernum;
            this.health = health;
            this.job = job;
            this.id = guid;
            this.possession = possession;
        }
    }
    public class Bullet  //子弹类
    {
        public int x; //横坐标(精细)
        public int y; //纵坐标(精细)
        public Int64 teamnum;
        public BulletType bulletType;
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
        public Int64 id;
        public Item(int xnum, int ynum, PropType type, Int64 guid)
        {
            this.xnum = xnum;
            this.ynum = ynum;
            this.type = (byte)type;
            this.id = guid;
        }
    }
    public class BoolLabel
    {
        public bool used;
        public Label label;
        public BoolLabel()
        {
            label = new Label();
            used = true;
        }
    }
    public class ClientData
    {
        public Int64 teamID;
        public Int64 playerID;
        public JobType jobType;
        public int bulletspeed;
        public ushort port;
        public Form1 gameform; //游戏窗体
        private Dictionary<Int64, Tuple<int, int>> Hashable;
        public Int64 selfguid;
        public bool searched = false;
        public BlockingCollection<Tuple<IMsg, long>> messageline;
        public ClientData(Int64 teamID, Int64 playerID, JobType jobType, ushort port)
        {
            this.teamID = teamID;
            this.playerID = playerID;
            this.jobType = jobType;
            this.port = port;
        }
        public void OnReciveShort(MessageToOneClient msg)  //连接是否成功
        {
            if (msg.MessageType == MessageType.ValidPlayer)
            {
            }
            else
            {
                MessageBox.Show("Invalid Player");
                Starting starting = new Starting();
                Application.Run(new Starting());
                MessageToServer msg1 = new MessageToServer();
                msg1.MessageType = MessageType.AddPlayer;
                msg1.TeamID = starting.teamid;
                msg1.PlayerID = starting.playerid;
                msg1.JobType = starting.job;
                Program.clientCommunicator.SendMessage(msg1);
            }
        }
        public void OnReciveNormal(MessageToClient msg, long clock) //处理游戏消息
        {
            switch (msg.MessageType)
            {
                case MessageType.StartGame:
                    Hashable = new Dictionary<Int64, Tuple<int, int>>();
                    for (int i = 0; i < msg.PlayerGUIDs.Count; i++)
                    {
                        for (int j = 0; j < msg.PlayerGUIDs[0].TeammateGUIDs.Count; j++)
                        {
                            Hashable.Add(msg.PlayerGUIDs[i].TeammateGUIDs[j], new Tuple<int, int>(i, j));
                        }
                    }
                    gameform.movespeed = msg.SelfInfo.MoveSpeed;
                    break;
                case MessageType.Gaming:
                    if (System.Environment.TickCount64 - clock > 50) break;
                    Refresh(msg);
                    break;
                case MessageType.EndGame:
                    Application.Exit();
                    break;
                default: break;
            }
        }
        private void Refresh(MessageToClient msg)  //刷新界面
        {
            gameform.selfguid = msg.SelfInfo.Guid;
            gameform.movespeed = msg.SelfInfo.MoveSpeed;
            for (int i = 0; i < 50; i++) //读取颜色
            {
                for (int j = 0; j < 50; j++)
                {
                    if (gameform.ColorState[i, j] == (int)msg.CellColors[j].RowColors[i])
                        gameform.ColorChange[i, j] = false;
                    else
                    {
                        gameform.ColorState[i, j] = (int)msg.CellColors[j].RowColors[i];
                        gameform.ColorChange[i, j] = true;
                    }
                }
            }
            foreach (var objinfo in msg.GameObjs)
            {
                Objdeal(objinfo);
            }
            gameform.Rebuild();
        }
        private void Objdeal(GameObjInfo obj)  //处理物体
        {
            switch (obj.GameObjType)
            {
                case GameObjType.BirthPoint:
                    if (gameform.ColorState[obj.Y / Program.cell, obj.X / Program.cell] == -2)
                        gameform.ColorChange[obj.Y / Program.cell, obj.X / Program.cell] = false;
                    else
                    {
                        switch (gameform.ColorState[obj.Y / Program.cell, obj.X / Program.cell])
                        {
                            case 1:   //队伍1:淡钢青色
                                gameform.Maplabels[obj.Y / Program.cell, obj.X / Program.cell].BackColor = System.Drawing.Color.LightSteelBlue;
                                break;
                            case 2:   //队伍2:淡绿色
                                gameform.Maplabels[obj.Y / Program.cell, obj.X / Program.cell].BackColor = System.Drawing.Color.LightGreen;
                                break;
                            case 3:   //队伍3:淡蓝色
                                gameform.Maplabels[obj.Y / Program.cell, obj.X / Program.cell].BackColor = System.Drawing.Color.LightBlue;
                                break;
                            case 4:   //队伍4:淡粉色
                                gameform.Maplabels[obj.Y / Program.cell, obj.X / Program.cell].BackColor = System.Drawing.Color.LightPink;
                                break;
                        }
                        gameform.ColorState[obj.Y / Program.cell, obj.X / Program.cell] = -2;
                        gameform.ColorChange[obj.Y / Program.cell, obj.X / Program.cell] = true;
                    }
                    break;
                case GameObjType.Wall:
                    if (gameform.ColorState[obj.Y / Program.cell, obj.X / Program.cell] == -1)
                        gameform.ColorChange[obj.Y / Program.cell, obj.X / Program.cell] = false;
                    else
                    {
                        gameform.ColorState[obj.Y / Program.cell, obj.X / Program.cell] = -1;
                        gameform.ColorChange[obj.Y / Program.cell, obj.X / Program.cell] = true;
                    }
                    break;
                case GameObjType.Character:
                    if (obj.IsDying) break;
                    Player player = new Player(obj.Y, obj.X, (byte)(obj.TeamID + 1), (byte)(Hashable[obj.Guid].Item2 + 1), obj.JobType, (short)obj.Hp, obj.Guid, obj.PropType);
                    gameform.DrawPlayer(player);
                    break;
                case GameObjType.Bullet:
                    Bullet bullet = new Bullet(obj.Y, obj.X, (int)obj.TeamID + 1, obj.BulletType, obj.Guid);
                    gameform.DrawBullet(bullet);
                    break;
                case GameObjType.Prop:
                    if (obj.IsLaid) break;
                    Item item = new Item(obj.Y / Program.cell, obj.X / Program.cell, obj.PropType, obj.Guid);
                    gameform.DrawItem(item);
                    break;
                default: break;
            }
        }
    }
}
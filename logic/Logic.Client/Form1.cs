using System;
using System.Collections.Generic;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;
using Communication.Proto;

namespace Logic.Client
{
    public partial class Form1 : Form
    {
        //字体大小
        static int fontforinfo = 20;  
        static int fontforexample = 13;
        static int fontforegnum = 15;
        static int fontforegpic = 14;
        static int fontforpic = 14;
        static int fontfornum = 13;
        static int fontforword = 15;
        static int fontforobjinfo = 15;
        HelpForm help = new HelpForm();
        private bool watch = false;
        public Form1(Int64 teamID, Int64 playerID, int bulletspeed, JobType job)  //窗体构造时的初始化
        {
            this.watch = (teamID == 1911 && playerID == 1911);
            this.teamid = teamID;
            this.playerid = playerID;
            this.bulletspeed = bulletspeed;
            InitializeComponent();
            this.ClientSize = new Size(50 * MapcellWidth + Interval + 250, 50 * MapcellHeight + 2 * Interval);
            this.AutoScroll = true;
            this.BackColor = Color.DimGray;
            this.MaximizeBox = false;
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            if (watch) this.Text = "Client---Watch Mode";
            else this.Text = "Client---TeamID: " + Convert.ToString(teamID) + " ---PlayerID: " + Convert.ToString(playerID) + " ---JobType: " + Convert.ToString(job);
            //this.SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            //this.SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
            //绘制地图
            for (int i = 0; i < 50; i++)
            {
                for (int j = 0; j < 50; j++)
                {
                    Maplabels[i, j] = new Label();
                    //设置大小位置
                    Maplabels[i, j].Size = new Size(MapcellWidth, MapcellHeight);
                    Maplabels[i, j].Location = new Point(i * MapcellWidth + Interval, j * MapcellHeight + Interval);
                    Maplabels[i, j].BackColor = Color.LightGray;
                    this.Controls.Add(Maplabels[i, j]);
                    int x = i * MapcellWidth + Interval, y = j * MapcellWidth + Interval;
                    Maplabels[i, j].Click += new EventHandler(LabelClick);
                    //Maplabels[i, j].Click += delegate(object sender, EventArgs e) { LabelClick(sender, e, x, y); }; ;
                }
            }
            //一大段绘制图例的语言,有空再缩减
            {
                Examplelabels[0] = new Label
                {
                    Size = new Size(MapcellWidth * 15, MapcellHeight * 2),
                    Location = new Point(Interval * 2 + 50 * MapcellWidth + 2, Interval / 2),
                    Text = "Examples",
                    TextAlign = ContentAlignment.MiddleLeft,
                    Font = new Font("等线", fontforinfo,GraphicsUnit.Pixel),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[1] = new Label
                {
                    BorderStyle = (BorderStyle)FormBorderStyle.FixedSingle,
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 50 * MapcellWidth + 7, Interval + MapcellHeight * 2 + 5),
                    BackColor = Color.DarkGray
                };
                Examplelabels[2] = new Label
                {
                    Size = new Size(MapcellWidth * 3 + 10, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 52 * MapcellWidth - 4, Interval + MapcellHeight * 2 + 6),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "出生点",
                    Font = new Font("等线", fontforexample, GraphicsUnit.Pixel),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[3] = new Label
                {
                    BorderStyle = (BorderStyle)FormBorderStyle.FixedSingle,
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 55 * MapcellWidth + 12, Interval + MapcellHeight * 2 + 5),
                    BackColor = Color.Brown
                };
                Examplelabels[4] = new Label
                {
                    Size = new Size(MapcellWidth * 3 + 10, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 57 * MapcellWidth + 1, Interval + MapcellHeight * 2 + 6),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "墙体",
                    Font = new Font("等线", fontforexample, GraphicsUnit.Pixel),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[5] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 50 * MapcellWidth + 7, Interval * 3 + MapcellHeight * 3),
                    BackColor = Color.LightSteelBlue
                };
                Examplelabels[6] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 52 * MapcellWidth + 1, Interval * 3 + MapcellHeight * 3),
                    BackColor = Color.LightGreen
                };
                Examplelabels[7] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 54 * MapcellWidth - 5, Interval * 3 + MapcellHeight * 3),
                    BackColor = Color.LightBlue
                };
                Examplelabels[8] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 56 * MapcellWidth - 11, Interval * 3 + MapcellHeight * 3),
                    BackColor = Color.LightPink
                };
                Examplelabels[9] = new Label
                {
                    Size = new Size(MapcellWidth * 7, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 58 * MapcellWidth - 20, Interval * 3 + MapcellHeight * 3 + 1),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "各队染色区域",
                    Font = new Font("等线", fontforexample, GraphicsUnit.Pixel),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[10] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 59 * MapcellWidth + 15, Interval + MapcellHeight * 2 + 5),
                    BackColor = Color.LightGray
                };
                Examplelabels[11] = new Label
                {
                    Size = new Size(MapcellWidth * 5 + 15, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 61 * MapcellWidth + 4, Interval + MapcellHeight * 2 + 6),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "未染色区域",
                    Font = new Font("等线", fontforexample, GraphicsUnit.Pixel),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[12] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "1",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", fontforegnum, GraphicsUnit.Pixel),
                    Location = new Point(Interval * 2 + 50 * MapcellWidth + 7, 96),
                    BackColor = Color.SteelBlue
                };
                Examplelabels[13] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "2",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", fontforegnum, GraphicsUnit.Pixel),
                    Location = new Point(Interval * 2 + 52 * MapcellWidth + 1, 96),
                    BackColor = Color.Green
                };
                Examplelabels[14] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "3",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", fontforegnum, GraphicsUnit.Pixel),
                    Location = new Point(Interval * 2 + 54 * MapcellWidth - 5, 96),
                    BackColor = Color.Blue
                };
                Examplelabels[15] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "4",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", fontforegnum, GraphicsUnit.Pixel),
                    Location = new Point(Interval * 2 + 56 * MapcellWidth - 11, 96),
                    BackColor = Color.Pink
                };
                Examplelabels[16] = new Label
                {
                    Size = new Size(MapcellWidth * 7, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 58 * MapcellWidth - 20, 96),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "各队玩家",
                    Font = new Font("等线", fontforexample, GraphicsUnit.Pixel),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[17] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "🏃",
                    ForeColor = Color.Red,
                    TextAlign = ContentAlignment.TopLeft,
                    Font = new Font("Times New Roman", fontforegpic, GraphicsUnit.Pixel),
                    Location = new Point(Interval * 2 + 50 * MapcellWidth + 7, 122),
                    BackColor = Color.Yellow
                };
                Examplelabels[18] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "❤",
                    ForeColor = Color.Red,
                    TextAlign = ContentAlignment.TopLeft,
                    Font = new Font("Times New Roman", fontforegpic, GraphicsUnit.Pixel),
                    Location = new Point(Interval * 2 + 52 * MapcellWidth + 1, 122),
                    BackColor = Color.Yellow
                };
                Examplelabels[19] = new Label
                {
                    Size = new Size(MapcellWidth * 7, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 54 * MapcellWidth - 5, 122),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "各类道具",
                    Font = new Font("等线", fontforexample, GraphicsUnit.Pixel),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                foreach (Label label in Examplelabels)
                {
                    label.Location = new Point(label.Location.X, label.Location.Y + 10);
                }
                for (int i = 0; i < 20; i++)
                {
                    this.Controls.Add(Examplelabels[i]);
                }
                Examplelabels[2].SendToBack();
                Examplelabels[4].SendToBack();
            }
            //地图信息绘制
            MapInfoPic.Size = new Size(MapcellWidth + 10, MapcellHeight + 10);
            MapInfoPic.Location = new Point(685, 245);
            MapInfoPic.BackColor = Color.Black;
            this.Controls.Add(MapInfoPic);
            MapInfoWord = new Label();
            MapInfoWord.Size = new Size(MapcellWidth * 14, MapcellHeight * 2);
            MapInfoWord.Location = new Point(710, 243);
            MapInfoWord.Text = "未被指定";
            MapInfoWord.TextAlign = ContentAlignment.MiddleLeft;
            MapInfoWord.Font = new Font("等线", fontforword, GraphicsUnit.Pixel);
            MapInfoWord.BackColor = Color.White;
            MapInfoWord.ForeColor = Color.Black;
            this.Controls.Add(MapInfoWord);
            MapInfo = new Label();
            MapInfo.Size = new Size(MapcellWidth * 10, MapcellHeight * 2);
            MapInfo.Location = new Point(672, 185);
            MapInfo.Text = "Map Info";
            MapInfo.TextAlign = ContentAlignment.MiddleLeft;
            MapInfo.Font = new Font("等线", fontforinfo, GraphicsUnit.Pixel);
            MapInfo.BackColor = Color.White;
            MapInfo.ForeColor = Color.Black;
            this.Controls.Add(MapInfo);
            //Object信息初始化
            for (int i = 0; i < 10; i++)
            {
                ObjectInfoWord[i] = new Label();
                ObjectInfoWord[i].TextAlign = ContentAlignment.MiddleLeft;
                ObjectInfoWord[i].BackColor = Color.White;
                ObjectInfoWord[i].ForeColor = Color.Black;
                this.Controls.Add(ObjectInfoWord[i]);
            }
            ObjectInfoWord[0].Size = new Size(MapcellWidth * 10, MapcellHeight * 2);
            ObjectInfoWord[0].Location = new Point(672, 328);
            ObjectInfoWord[0].Text = "Object Info";
            ObjectInfoWord[0].Font = new Font("等线", fontforinfo, GraphicsUnit.Pixel);
            for (int i = 1; i < 10; i++)
            {
                ObjectInfoWord[i].Location = new Point(672, 340 + i * 2 * MapcellHeight);
                ObjectInfoWord[i].Font = new Font("等线", fontforobjinfo, GraphicsUnit.Pixel);
                ObjectInfoWord[i].Size = new Size(MapcellWidth * 16, MapcellHeight + 3);
            }
            ObjectInfoWord[9].Size = new Size(MapcellWidth * 16, 3 * MapcellHeight);
            ObjectInfoWord[9].TextAlign = ContentAlignment.TopLeft;
            //玩家示例代码
            //子弹示例代码
            //道具示例代码
            //new Thread(() =>
            //{
            //    while (true)
            //    {
            //        this.Refresh();
            //        Thread.Sleep(2000);
            //    }
            //}
            //    ).Start();            
        }
        protected readonly int MapcellWidth = 13;  //单元格长度
        protected readonly int MapcellHeight = 13;  //单元格高度
        protected readonly int Interval = 10;  //单元格间距
        public Label[,] Maplabels = new Label[50, 50];  //地图
        protected Label[] Examplelabels = new Label[20];  //图例        
        //显示地图信息
        protected Label MapInfoPic = new Label();
        protected Label MapInfoWord = new Label();
        protected Label MapInfo = new Label();
        //显示物品信息
        protected Label ObjectInfo = new Label();
        protected Label[] ObjectInfoWord = new Label[10];
        //绘制窗体
        private void Form1_Paint(object sender, PaintEventArgs e)
        {
            Graphics Beatify = this.CreateGraphics();
            Pen Magicpen = new Pen(Color.WhiteSmoke, 2);
            Rectangle r1 = new Rectangle(Interval / 2 + 3, Interval / 2 + 3, Interval + 50 * MapcellWidth - 6, Interval + 50 * MapcellHeight - 6);
            Rectangle r2 = new Rectangle(Interval * 2 + 50 * MapcellWidth, Interval / 2 + 3, 18 * MapcellWidth, 12 * MapcellHeight);
            Rectangle r3 = new Rectangle(Interval * 2 + 50 * MapcellWidth + 2, Interval / 2 + 5, 18 * MapcellWidth - 4, 12 * MapcellHeight - 4);
            Beatify.DrawRectangle(Magicpen, r1);
            Magicpen = new Pen(Color.WhiteSmoke, 2);
            Beatify.DrawRectangle(Magicpen, r2);
            Brush Magicbrush = new SolidBrush(Color.White);
            Beatify.FillRectangle(Magicbrush, r3);
            Magicpen.Dispose();
            r2 = new Rectangle(Interval * 2 + 50 * MapcellWidth, Interval / 2 + 3 + 13 * MapcellHeight, 18 * MapcellWidth, 10 * MapcellHeight);
            r3 = new Rectangle(Interval * 2 + 50 * MapcellWidth + 2, Interval / 2 + 5 + 13 * MapcellHeight, 18 * MapcellWidth - 4, 10 * MapcellHeight - 4);
            Magicpen = new Pen(Color.WhiteSmoke, 2);
            Beatify.DrawRectangle(Magicpen, r2);
            Magicbrush = new SolidBrush(Color.White);
            Beatify.FillRectangle(Magicbrush, r3);
            Magicpen.Dispose();
            r2 = new Rectangle(Interval * 2 + 50 * MapcellWidth, Interval / 2 + 3 + 24 * MapcellHeight, 18 * MapcellWidth, 26 * MapcellHeight + 4);
            r3 = new Rectangle(Interval * 2 + 50 * MapcellWidth + 2, Interval / 2 + 5 + 24 * MapcellHeight, 18 * MapcellWidth - 4, 26 * MapcellHeight);
            Magicpen = new Pen(Color.WhiteSmoke, 2);
            Beatify.DrawRectangle(Magicpen, r2);
            Magicbrush = new SolidBrush(Color.White);
            Beatify.FillRectangle(Magicbrush, r3);
            Magicpen.Dispose();
        }
        //动态标签集合
        Dictionary<long, BoolLabel> PlayerLabelSet = new Dictionary<long, BoolLabel>();
        Dictionary<long, BoolLabel> BulletLabelSet = new Dictionary<long, BoolLabel>();
        Dictionary<long, BoolLabel> ItemLabelSet = new Dictionary<long, BoolLabel>();
        public int[,] ColorState = new int[50, 50];  //储存每个地图格的染色状态 0:未被染色 i:第i队染色 -1:墙体 -2:出生点
        public bool[,] ColorChange = new bool[50, 50];  //储存每个地图格的染色状态 0:未被染色 i:第i队染色 -1:墙体 -2:出生点
        public Int64 teamid;
        public Int64 playerid;
        public Int64 selfguid;
        public int movespeed;
        public int bulletspeed;
        public void DrawPlayer(Player player)  //绘制玩家
        {
            this.Invoke(new Action(() =>
            {
                if (PlayerLabelSet.ContainsKey(player.id))
                {
                    PlayerLabelSet[player.id].label.Location = new Point((player.x * MapcellWidth + Program.cell / 2) / Program.cell - (MapcellWidth + 1) / 2 + Interval, (player.y * MapcellHeight + Program.cell / 2) / Program.cell - (MapcellHeight + 1) / 2 + Interval);
                    PlayerLabelSet[player.id].label.BringToFront();
                    PlayerLabelSet[player.id].used = true;
                    PlayerLabelSet[player.id].label.Click += delegate (object sender, EventArgs e) { PlayerClick(sender, e, player); }; ;
                }
                else
                {
                    BoolLabel label = new BoolLabel();
                    label.label.Size = new Size(MapcellWidth + 1, MapcellHeight + 1);
                    label.label.Location = new Point((player.x * MapcellWidth + Program.cell / 2) / Program.cell - (MapcellWidth + 1) / 2 + Interval, (player.y * MapcellHeight + Program.cell / 2) / Program.cell - (MapcellHeight + 1) / 2 + Interval);
                    label.label.Text = Convert.ToString(player.playernum);
                    label.label.ForeColor = Color.White;
                    label.label.TextAlign = ContentAlignment.TopRight;
                    label.label.Font = new Font("Times New Roman", fontfornum, GraphicsUnit.Pixel);
                    label.label.Click += delegate (object sender, EventArgs e) { PlayerClick(sender, e, player); }; ;
                    switch (player.teamnum)
                    {
                        case 1:
                            label.label.BackColor = Color.SteelBlue;
                            break;
                        case 2:
                            label.label.BackColor = Color.Green;
                            break;
                        case 3:
                            label.label.BackColor = Color.Blue;
                            break;
                        case 4:
                            label.label.BackColor = Color.Pink;
                            break;
                    }
                    this.Controls.Add(label.label);
                    label.label.BringToFront();
                    PlayerLabelSet.Add(player.id, label);
                }
            }));
        }
        public void DrawBullet(Bullet bullet)  //绘制子弹
        {
            this.Invoke(new Action(() =>
            {
                if (BulletLabelSet.ContainsKey(bullet.id))
                {
                    BulletLabelSet[bullet.id].label.Location = new Point((bullet.x * MapcellWidth + Program.cell / 2) / Program.cell - (MapcellWidth + 1) / 2 + Interval, (bullet.y * MapcellHeight + Program.cell / 2) / Program.cell - (MapcellHeight + 1) / 2 + Interval);
                    BulletLabelSet[bullet.id].label.BringToFront();
                    BulletLabelSet[bullet.id].used = true;
                }
                else
                {
                    BoolLabel label = new BoolLabel();
                    label.label.Size = new Size(5, 5);
                    label.label.Location = new Point((bullet.x * MapcellWidth + Program.cell / 2) / Program.cell - (MapcellWidth + 1) / 2 + Interval, (bullet.y * MapcellHeight + Program.cell / 2) / Program.cell - (MapcellHeight + 1) / 2 + Interval);  //或许会再修改
                    switch (bullet.teamnum)
                    {
                        case 1:
                            label.label.BackColor = Color.SteelBlue;
                            break;
                        case 2:
                            label.label.BackColor = Color.Green;
                            break;
                        case 3:
                            label.label.BackColor = Color.Blue;
                            break;
                        case 4:
                            label.label.BackColor = Color.Pink;
                            break;
                    }
                    this.Controls.Add(label.label);
                    label.label.BringToFront();
                    BulletLabelSet.Add(bullet.id, label);
                }
            }));
        }
        public void DrawItem(Item item)  //绘制道具
        {
            this.Invoke(new Action(() =>
            {
                if (ItemLabelSet.ContainsKey(item.id))
                {
                    ItemLabelSet[item.id].label.Location = new Point(item.xnum * MapcellWidth + Interval, item.ynum * MapcellHeight + Interval);
                    ItemLabelSet[item.id].label.BringToFront();
                    ItemLabelSet[item.id].used = true;
                }
                else
                {
                    BoolLabel label = new BoolLabel();
                    label.label.Location = new Point(item.xnum * MapcellWidth + Interval, item.ynum * MapcellHeight + Interval);  //或许会再修改
                    label.label.Size = new Size(MapcellWidth, MapcellHeight);
                    label.label.ForeColor = Color.Red;
                    label.label.TextAlign = ContentAlignment.TopCenter;
                    label.label.Font = new Font("Times New Roman", fontforpic, GraphicsUnit.Pixel);
                    label.label.BackColor = Color.Yellow;
                    switch (item.type)
                    {
                        case 0:
                            break;
                        case 1:
                            label.label.Text = "🏃";
                            break;
                        case 2:
                            label.label.Text = "⚔";
                            break;
                        case 3:
                            label.label.Text = "⭯";
                            break;
                        case 4:
                            label.label.Text = "❤";
                            break;
                        case 5:
                            label.label.Text = "⛨";
                            break;
                        case 6:
                            label.label.Text = "⍏";
                            break;
                        case 7:
                            label.label.Text = "❎";
                            break;
                        case 8:
                        case 9:
                        case 10:
                            label.label.Text = "🚩";
                            break;
                    }
                    label.label.Click += delegate (object sender, EventArgs e) { ItemClick(sender, e, item); }; ;
                    this.Controls.Add(label.label);
                    label.label.BringToFront();
                    ItemLabelSet.Add(item.id, label);
                }
            }));
        }
        public void changeform(Int64 teamID,Int64 playerID,JobType jobType)
        {
            this.Invoke(new Action(() =>
            {
                this.Text = "Client--- TeamID: " + Convert.ToString(teamID) + " ---PlayerID: " + Convert.ToString(playerID) + " ---JobType: " + Convert.ToString(jobType);
                this.playerid = playerID;
                this.teamid = teamID;
            }));
        }
        private void LabelClick(object sender, EventArgs e)  //标签单击事件处理
        {
            MouseEventArgs mouseEventArgs = (MouseEventArgs)e;
            Label label = sender as Label;
            if (mouseEventArgs.Button == MouseButtons.Left)
            {
                int x = label.Location.X + mouseEventArgs.X;
                int y = label.Location.Y + mouseEventArgs.Y;
                if (x < Interval || x > Interval + 50 * MapcellWidth) return;
                if (y < Interval || y > Interval + 50 * MapcellHeight) return;
                int xnum = (x - Interval) / MapcellWidth;
                int ynum = (y - Interval) / MapcellHeight;
                int colorstate = ColorState[xnum, ynum];
                switch (colorstate)
                {
                    case -2:  //出生点:深灰色+边框
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.FixedSingle;
                        MapInfoPic.BackColor = Color.DarkGray;
                        MapInfoWord.Text = "这是出生点";
                        break;
                    case -1:  //墙体:黑色+边框
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.FixedSingle;
                        MapInfoPic.BackColor = Color.Brown;
                        MapInfoWord.Text = "这是墙体";
                        break;
                    case 0:   //未染色区域:淡灰
                        MapInfoPic.BackColor = Color.LightGray;
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.None;
                        MapInfoWord.Text = "这是未染色区域";
                        break;
                    case 1:   //队伍1:淡钢青色
                        MapInfoPic.BackColor = Color.LightSteelBlue;
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.None;
                        MapInfoWord.Text = "这是队伍1染色区域";
                        break;
                    case 2:   //队伍2:淡绿色
                        MapInfoPic.BackColor = Color.LightGreen;
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.None;
                        MapInfoWord.Text = "这是队伍2染色区域";
                        break;
                    case 3:   //队伍3:淡蓝色
                        MapInfoPic.BackColor = Color.LightBlue;
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.None;
                        MapInfoWord.Text = "这是队伍3染色区域";
                        break;
                    case 4:   //队伍4:淡粉色
                        MapInfoPic.BackColor = Color.LightPink;
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.None;
                        MapInfoWord.Text = "这是队伍4染色区域";
                        break;
                }
            }
            else if (mouseEventArgs.Button == MouseButtons.Right&&!watch)
            {
                int y = this.PointToClient(Control.MousePosition).X;
                int x = this.PointToClient(Control.MousePosition).Y;
                MessageToServer msg = new MessageToServer();
                msg.PlayerID = playerid;
                msg.TeamID = teamid;
                msg.MessageType = MessageType.Move;
                msg.Angle = Math.Atan2(y - PlayerLabelSet[selfguid].label.Location.X - 7, x - PlayerLabelSet[selfguid].label.Location.Y - 6);//目前为弧度
                msg.TimeInMilliseconds = (int)(1000.0 * Math.Sqrt(Math.Pow((double)(y - PlayerLabelSet[selfguid].label.Location.X - 7), 2) + Math.Pow((double)(x - PlayerLabelSet[selfguid].label.Location.Y - 6), 2)) * Program.cell / MapcellHeight / movespeed + 0.5);
                //TO DO:向server发移动指令消息
                Program.clientCommunicator.SendMessage(msg);
            }
        }
        private void PlayerClick(object sender, EventArgs e, Player player) //玩家点击事件处理
        {
            LabelClick(sender, e); //标签点击通用处理
            ObjectInfoWord[1].Text = "ObjectType : Player";
            ObjectInfoWord[2].Text = "TeamNum : " + Convert.ToString(player.teamnum);
            ObjectInfoWord[3].Text = "PlayerNum : " + Convert.ToString(player.playernum);
            ObjectInfoWord[4].Text = "Profession : " + Convert.ToString(player.job);
            ObjectInfoWord[5].Text = "X : " + Convert.ToString(player.y / Program.cell) + " (+" + Convert.ToString(player.x % Program.cell) + ")";
            ObjectInfoWord[6].Text = "Y : " + Convert.ToString(player.x / Program.cell) + " (+" + Convert.ToString(player.y % Program.cell) + ")";
            ObjectInfoWord[7].Text = "Health : " + Convert.ToString(player.health);
            String tempo = "";
            switch ((int)player.possession)
            {
                case 0:
                    tempo = "无道具";
                    break;
                case 1:
                    tempo = "增加移动速度";
                    break;
                case 2:
                    tempo = "增加攻击伤害";
                    break;
                case 3:
                    tempo = "减少换弹CD";
                    break;
                case 4:
                    tempo = "回复一定血量";
                    break;
                case 5:
                    tempo = "提供护盾";
                    break;
                case 6:
                    tempo = "提供复活甲";
                    break;
                case 7:
                    tempo = "穿甲弓 (即破盾)";
                    break;
                case 8:
                    tempo = "地雷 (减速)";
                    break;
                case 9:
                    tempo = "地雷 (减伤)";
                    break;
                case 10:
                    tempo = "地雷 (增加换弹CD)";
                    break;
                default: break;
            }
            ObjectInfoWord[8].Text = "Possession : " + tempo;
        }
        private void ItemClick(object sender, EventArgs e, Item item)
        {
            LabelClick(sender, e); //标签点击通用处理
            ObjectInfoWord[1].Text = "ObjectType : Item";
            ObjectInfoWord[2].Text = "X : " + Convert.ToString(item.ynum) + " (范围:0-49)";
            ObjectInfoWord[3].Text = "Y : " + Convert.ToString(item.xnum) + " (范围:0-49)";
            String tempo = "";
            switch (item.type)
            {
                case 0:
                    tempo = "";
                    break;
                case 1:
                    tempo = "增加移动速度";
                    break;
                case 2:
                    tempo = "增加攻击伤害";
                    break;
                case 3:
                    tempo = "减少换弹CD";
                    break;
                case 4:
                    tempo = "回复一定血量";
                    break;
                case 5:
                    tempo = "提供护盾";
                    break;
                case 6:
                    tempo = "提供复活甲";
                    break;
                case 7:
                    tempo = "穿甲弓 (即破盾)";
                    break;
                case 8:
                    tempo = "地雷 (减速)";
                    break;
                case 9:
                    tempo = "地雷 (减伤)";
                    break;
                case 10:
                    tempo = "地雷 (增加换弹CD)";
                    break;
                default: break;
            }
            ObjectInfoWord[4].Text = "Fuction : " + tempo;
            for (int i = 5; i < 10; i++)
            {
                ObjectInfoWord[i].Text = "";
            }
        }
        private void Form1_KeyPress(object sender, KeyPressEventArgs e) //键盘进行特殊操作
        {
            if (watch) return;
            else
            {
                switch (e.KeyChar)
                {
                    case 'q':
                    case 'Q':
                        int y = this.PointToClient(Control.MousePosition).X;
                        int x = this.PointToClient(Control.MousePosition).Y;
                        MessageToServer msg1 = new MessageToServer();
                        msg1.PlayerID = playerid;
                        msg1.TeamID = teamid;
                        msg1.MessageType = MessageType.Attack;
                        msg1.Angle = Math.Atan2(y - PlayerLabelSet[selfguid].label.Location.X, x - PlayerLabelSet[selfguid].label.Location.Y);//目前为弧度
                        msg1.TimeInMilliseconds = (int)(1000.0 * Math.Sqrt(Math.Pow((double)(y - PlayerLabelSet[selfguid].label.Location.X - 5), 2) + Math.Pow((double)(x - PlayerLabelSet[selfguid].label.Location.Y - 5), 2)) * Program.cell / MapcellHeight / bulletspeed + 0.5);
                        Program.clientCommunicator.SendMessage(msg1);
                        break;
                    case 'w':
                    case 'W':
                        MessageToServer msg2 = new MessageToServer();
                        msg2.MessageType = MessageType.Use;
                        msg2.PlayerID = playerid;
                        msg2.TeamID = teamid;
                        //TO DO:发消息
                        Program.clientCommunicator.SendMessage(msg2);
                        break;
                    case 'e':
                    case 'E':
                        MessageToServer msg3 = new MessageToServer();
                        msg3.MessageType = MessageType.Pick;
                        msg3.PlayerID = playerid;
                        msg3.TeamID = teamid;
                        //TO DO:发消息
                        for (int i = 1; i < 11; i++)
                        {
                            msg3.PropType = (PropType)i;
                            Program.clientCommunicator.SendMessage(msg3);
                        }
                        break;
                    case 'r':
                    case 'R':
                        MessageToServer msg4 = new MessageToServer();
                        msg4.MessageType = MessageType.Throw;
                        msg4.PlayerID = playerid;
                        msg4.TeamID = teamid;
                        //TO DO:发消息
                        Program.clientCommunicator.SendMessage(msg4);
                        break;
                    default: break;
                }
            }
        }
        //地雷先不画出“🚩”——锅
        public void Rebuild()  //刷新界面
        {
            this.Invoke(new Action(() =>
            {
                foreach (var item in PlayerLabelSet)
                {
                    if (item.Value.used)
                    {
                        item.Value.used = false;
                    }
                    else
                    {
                        this.Controls.Remove(item.Value.label);
                        PlayerLabelSet.Remove(item.Key);
                    }
                }
                foreach (var item in BulletLabelSet)
                {
                    if (item.Value.used)
                    {
                        item.Value.used = false;
                    }
                    else
                    {
                        this.Controls.Remove(item.Value.label);
                        BulletLabelSet.Remove(item.Key);
                    }
                }
                foreach (var item in ItemLabelSet)
                {
                    if (item.Value.used)
                    {
                        item.Value.used = false;
                    }
                    else
                    {
                        this.Controls.Remove(item.Value.label);
                        ItemLabelSet.Remove(item.Key);
                    }
                }
                Redraw(ColorState);
            }));
        }
        public void Redraw(int[,] vs) //重绘地图
        {
            for (int i = 0; i < 50; i++)
            {
                for (int j = 0; j < 50; j++)
                {
                    if (ColorChange[i, j])
                        switch (vs[i, j])
                        {
                            case -2:  //出生点:深灰色+边框
                                Maplabels[i, j].BorderStyle = (BorderStyle)FormBorderStyle.FixedSingle;
                                break;
                            case -1:  //墙体:黑色+边框
                                Maplabels[i, j].BorderStyle = (BorderStyle)FormBorderStyle.FixedSingle;
                                Maplabels[i, j].BackColor = Color.Brown;
                                break;
                            case 0:   //未染色区域:淡灰
                                Maplabels[i, j].BackColor = Color.LightGray;
                                break;
                            case 1:   //队伍1:淡钢青色
                                Maplabels[i, j].BackColor = Color.LightSteelBlue;
                                break;
                            case 2:   //队伍2:淡绿色
                                Maplabels[i, j].BackColor = Color.LightGreen;
                                break;
                            case 3:   //队伍3:淡蓝色
                                Maplabels[i, j].BackColor = Color.LightBlue;
                                break;
                            case 4:   //队伍4:淡粉色
                                Maplabels[i, j].BackColor = Color.LightPink;
                                break;
                        }
                }
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            Application.Exit();
        }

        private void Form1_HelpButtonClicked(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (watch) 
            { 
                MessageBox.Show("此为观战模式，仅可左键查看信息"); 
                this.WindowState = FormWindowState.Minimized;
            }
            else
            {
                help = new HelpForm();
                help.Show();
                this.WindowState = FormWindowState.Minimized;
            }
        }
    }
}

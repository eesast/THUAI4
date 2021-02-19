using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace Logic.Client
{
    public partial class Form1 : Form
    {
        public Form1()  //窗体构造时的初始化
        {
            InitializeComponent();
            this.ClientSize = new Size(50 * MapcellWidth + Interval + 250, 50 * MapcellHeight + 2 * Interval);
            this.AutoScroll = true;
            this.BackColor = Color.DimGray;
            this.MaximizeBox = false;
            this.StartPosition = FormStartPosition.CenterScreen;
            this.FormBorderStyle = FormBorderStyle.FixedDialog;
            this.Text = "Client---简易调试界面";
            //绘制地图
            for (int i = 0; i < 50; i++)
            {
                for (int j = 0; j < 50; j++)
                {
                    Maplabels[i, j] = new Label();
                    //设置大小位置
                    Maplabels[i, j].Size = new Size(MapcellWidth, MapcellHeight);
                    Maplabels[i, j].Location = new Point(i * MapcellWidth + Interval, j * MapcellHeight + Interval);
                    switch (Program.GetColorState(i, j))
                    {
                        case -2:  //出生点:深灰色+边框
                            Maplabels[i, j].BackColor = Color.DarkGray;
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
                    Font = new Font("等线", 17),
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
                    Font = new Font("等线", 10),
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
                    Font = new Font("等线", 10),
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
                    Font = new Font("等线", 10),
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
                    Font = new Font("等线", 10),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[12] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "1",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", 10),
                    Location = new Point(Interval * 2 + 50 * MapcellWidth + 7, 96),
                    BackColor = Color.SteelBlue
                };
                Examplelabels[13] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "2",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", 10),
                    Location = new Point(Interval * 2 + 52 * MapcellWidth + 1, 96),
                    BackColor = Color.Green
                };
                Examplelabels[14] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "3",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", 10),
                    Location = new Point(Interval * 2 + 54 * MapcellWidth - 5, 96),
                    BackColor = Color.Blue
                };
                Examplelabels[15] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "4",
                    ForeColor = Color.White,
                    TextAlign = ContentAlignment.TopRight,
                    Font = new Font("Times New Roman", 10),
                    Location = new Point(Interval * 2 + 56 * MapcellWidth - 11, 96),
                    BackColor = Color.Pink
                };
                Examplelabels[16] = new Label
                {
                    Size = new Size(MapcellWidth * 7, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 58 * MapcellWidth - 20, 96),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "各队玩家",
                    Font = new Font("等线", 10),
                    BackColor = Color.White,
                    ForeColor = Color.Black
                };
                Examplelabels[17] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "🏃",
                    ForeColor = Color.Red,
                    TextAlign = ContentAlignment.TopLeft,
                    Font = new Font("Times New Roman", 11),
                    Location = new Point(Interval * 2 + 50 * MapcellWidth + 7, 122),
                    BackColor = Color.Yellow
                };
                Examplelabels[18] = new Label
                {
                    Size = new Size(MapcellWidth + 3, MapcellHeight + 3),
                    Text = "❤",
                    ForeColor = Color.Red,
                    TextAlign = ContentAlignment.TopLeft,
                    Font = new Font("Times New Roman", 11),
                    Location = new Point(Interval * 2 + 52 * MapcellWidth + 1, 122),
                    BackColor = Color.Yellow
                };
                Examplelabels[19] = new Label
                {
                    Size = new Size(MapcellWidth * 7, MapcellHeight + 3),
                    Location = new Point(Interval * 2 + 54 * MapcellWidth - 5, 122),
                    TextAlign = ContentAlignment.MiddleLeft,
                    Text = "各类道具",
                    Font = new Font("等线", 10),
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
            MapInfoWord.Font = new Font("等线", 13);
            MapInfoWord.BackColor = Color.White;
            MapInfoWord.ForeColor = Color.Black;
            this.Controls.Add(MapInfoWord);
            MapInfo = new Label();
            MapInfo.Size = new Size(MapcellWidth * 10, MapcellHeight * 2);
            MapInfo.Location = new Point(672, 185);
            MapInfo.Text = "Map Info";
            MapInfo.TextAlign = ContentAlignment.MiddleLeft;
            MapInfo.Font = new Font("等线", 17);
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
            ObjectInfoWord[0].Font = new Font("等线", 17);
            for (int i = 1; i < 10; i++)
            {
                ObjectInfoWord[i].Location = new Point(672, 340 + i * 2 * MapcellHeight);
                ObjectInfoWord[i].Font = new Font("等线", 10);
                ObjectInfoWord[i].Size = new Size(MapcellWidth * 16, MapcellHeight + 3);
            }
            ObjectInfoWord[9].Size = new Size(MapcellWidth * 16, 3 * MapcellHeight);
            ObjectInfoWord[9].TextAlign = ContentAlignment.TopLeft;
            //玩家示例代码
            Player player1 = new Player(10256, 1800, 1, 1, (Player.Job)1, 100);
            Player player2 = new Player(3000, 400, 3, 3, (Player.Job)2, 1000);
            player1.stunned = 1;
            player2.stunned = 1; player2.speed = 2; player2.damage = 1; player2.shield = 2;
            player2.possession = Player.Possession.Shorter_Reload_CD;
            DrawPlayer(player1);
            DrawPlayer(player2);
            //子弹示例代码
            Bullet bullet1 = new Bullet(60, 60, 2, true);
            DrawBullet(bullet1);
            //道具示例代码
            Item item1 = new Item(15, 15, 1);
            Item item2 = new Item(28, 30, 10);
            Item item3 = new Item(7, 16, 0);

            DrawItem(item1); DrawItem(item2);
        }
        protected readonly int MapcellWidth = 13;  //单元格长度
        protected readonly int MapcellHeight = 13;  //单元格高度
        protected readonly int Interval = 10;  //单元格间距
        protected Label[,] Maplabels = new Label[50, 50];  //地图
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
        Dictionary<long, Label> PlayerLabelSet = new Dictionary<long, Label>();
        Dictionary<long, Label> BulletLabelSet = new Dictionary<long, Label>();
        Dictionary<long, Label> ItemLabelSet = new Dictionary<long, Label>();
        private void DrawPlayer(Player player)  //绘制玩家
        {
            if (player.existed)
            {
                PlayerLabelSet[player.teamnum * 10 + player.playernum].Location = new Point(player.x, player.y);
                PlayerLabelSet[player.teamnum * 10 + player.playernum].BringToFront();
            }
            else
            {
                Label label = new Label();
                label.Size = new Size(MapcellWidth + 1, MapcellHeight + 1);
                label.Location = new Point((player.x * MapcellWidth + 512) / 1024 - (MapcellWidth + 1) / 2 + Interval, (player.y * MapcellHeight + 512) / 1024 - (MapcellHeight + 1) / 2 + Interval);
                label.Text = Convert.ToString(player.playernum);
                label.ForeColor = Color.White;
                label.TextAlign = ContentAlignment.TopRight;
                label.Font = new Font("Times New Roman", 8);
                label.Click += delegate (object sender, EventArgs e) { PlayerClick(sender, e, player); }; ;
                switch (player.teamnum)
                {
                    case 1:
                        label.BackColor = Color.SteelBlue;
                        break;
                    case 2:
                        label.BackColor = Color.Green;
                        break;
                    case 3:
                        label.BackColor = Color.Blue;
                        break;
                    case 4:
                        label.BackColor = Color.Pink;
                        break;
                }
                this.Controls.Add(label);
                label.BringToFront();
                PlayerLabelSet.Add(player.teamnum * 10 + player.playernum, label);
                player.existed = true;
            }
        }
        private void DrawBullet(Bullet bullet)  //绘制子弹
        {
            if (bullet.existed)
            {
                BulletLabelSet[bullet.id].Location = new Point(bullet.x, bullet.y);
                BulletLabelSet[bullet.id].BringToFront();
            }
            else
            {
                Label label = new Label();
                if (bullet.towards)
                {
                    label.Size = new Size(5, 3);
                }
                else
                {
                    label.Size = new Size(3, 5);
                }
                label.Location = new Point(bullet.x, bullet.y);  //或许会再修改
                switch (bullet.teamnum)
                {
                    case 1:
                        label.BackColor = Color.SteelBlue;
                        break;
                    case 2:
                        label.BackColor = Color.Green;
                        break;
                    case 3:
                        label.BackColor = Color.Blue;
                        break;
                    case 4:
                        label.BackColor = Color.Pink;
                        break;
                }
                this.Controls.Add(label);
                label.BringToFront();
                BulletLabelSet.Add(bullet.id, label);
                bullet.existed = true;
            }
        }
        private void DrawItem(Item item)  //绘制道具
        {
            if (item.existed)
            {
                ItemLabelSet[item.id].Location = new Point(item.xnum * MapcellWidth + Interval, item.ynum * MapcellHeight + Interval);
                ItemLabelSet[item.id].BringToFront();
            }
            else
            {
                Label label = new Label
                {
                    Location = new Point(item.xnum * MapcellWidth + Interval, item.ynum * MapcellHeight + Interval),  //或许会再修改
                    Size = new Size(MapcellWidth, MapcellHeight),
                    ForeColor = Color.Red,
                    TextAlign = ContentAlignment.TopCenter,
                    Font = new Font("Times New Roman", 9),
                    BackColor = Color.Yellow
                };
                switch (item.type)
                {
                    case 0:
                        label.Text = "🏃";
                        break;
                    case 1:
                        label.Text = "⚔";
                        break;
                    case 2:
                        label.Text = "⭯";
                        break;
                    case 3:
                        label.Text = "❤";
                        break;
                    case 4:
                        label.Text = "⛨";
                        break;
                    case 5:
                        label.Text = "⍏";
                        break;
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                    case 10:
                        label.Text = "🚩";
                        break;
                    case 11:
                        label.Text = "❎";
                        break;
                }
                label.Click += delegate (object sender, EventArgs e) { ItemClick(sender, e, item); }; ;
                this.Controls.Add(label);
                label.BringToFront();
                ItemLabelSet.Add(item.id, label);
                item.existed = true;
            }
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
                int colorstate = Program.GetColorState(xnum, ynum);
                switch (colorstate)
                {
                    case -2:  //出生点:深灰色+边框
                        MapInfoPic.BackColor = Color.DarkGray;
                        MapInfoPic.BorderStyle = (BorderStyle)FormBorderStyle.FixedSingle;
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
            else if (mouseEventArgs.Button == MouseButtons.Right)
            {
                int x = label.Location.X + mouseEventArgs.X - Interval;
                int y = label.Location.Y + mouseEventArgs.Y - Interval;
                //TO DO:向server发移动指令的消息
            }
        }
        private void PlayerClick(object sender, EventArgs e, Player player) //玩家点击事件处理
        {
            LabelClick(sender, e); //标签点击通用处理
            ObjectInfoWord[1].Text = "ObjectType : Player";
            ObjectInfoWord[2].Text = "TeamNum : " + Convert.ToString(player.teamnum);
            ObjectInfoWord[3].Text = "PlayerNum : " + Convert.ToString(player.playernum);
            ObjectInfoWord[4].Text = "Profession : " + Convert.ToString(player.job);
            ObjectInfoWord[5].Text = "X : " + Convert.ToString(player.x / 1024) + " (+" + Convert.ToString(player.x % 1024) + ")";
            ObjectInfoWord[6].Text = "Y : " + Convert.ToString(player.y / 1024) + " (+" + Convert.ToString(player.y % 1024) + ")";
            ObjectInfoWord[7].Text = "Health : " + Convert.ToString(player.health);
            ObjectInfoWord[8].Text = "Possession : " + Convert.ToString(player.possession);
            String tempo = "";
            switch (player.shield)
            {
                case 0: break;
                case 1: tempo += "/Shield "; break;
                case 2: tempo += "/Revive "; break;
                default: break;
            }
            switch (player.speed)
            {
                case 0: break;
                case 1: tempo += "/SpeedUp "; break;
                case 2: tempo += "/SpeedDown "; break;
                default: break;
            }
            switch (player.damage)
            {
                case 0: break;
                case 1: tempo += "/DamageUp "; break;
                case 2: tempo += "/DamageDown "; break;
                default: break;
            }
            switch (player.stunned)
            {
                case 0: break;
                case 1: tempo += "/Stunned "; break;
                default: break;
            }
            ObjectInfoWord[9].Text = "Status : " + tempo;
        }
        private void ItemClick(object sender, EventArgs e, Item item)
        {
            LabelClick(sender, e); //标签点击通用处理
            ObjectInfoWord[1].Text = "ObjectType : Item";
            ObjectInfoWord[2].Text = "X : " + Convert.ToString(item.xnum) + " (范围:0-49)";
            ObjectInfoWord[3].Text = "Y : " + Convert.ToString(item.ynum) + " (范围:0-49)";
            String tempo = "";
            switch (item.type)
            {
                case 0:
                    tempo = "增加移动速度";
                    break;
                case 1:
                    tempo = "增加攻击伤害";
                    break;
                case 2:
                    tempo = "减少换弹CD";
                    break;
                case 3:
                    tempo = "回复一定血量";
                    break;
                case 4:
                    tempo = "提供护盾";
                    break;
                case 5:
                    tempo = "提供复活甲";
                    break;
                case 6:
                    tempo = "地雷 (减速)";
                    break;
                case 7:
                    tempo = "地雷 (减伤)";
                    break;
                case 8:
                    tempo = "地雷 (增加换弹CD)";
                    break;
                case 9:
                    tempo = "地雷 (具有一定伤害)";
                    break;
                case 10:
                    tempo = "地雷 (眩晕)";
                    break;
                case 11:
                    tempo = "穿甲弓 (即破盾)";
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
            switch (e.KeyChar)
            {
                case 'q':
                case 'Q':
                //TO DO:开火
                case 'w':
                case 'W':
                //TO DO:使用道具
                case 'e':
                case 'E':
                //TO DO:捡起道具
                case 'r':
                case 'R':
                //TO DO:选择职业
                default: break;
            }
        }
        //地雷先不画出“🚩”——锅
    }
}

using System;
using System.Linq;
using Communication.CommServer;
using Communication.Proto;
using System.Threading;
using Google.Protobuf;
namespace servertest
{
    class Test
    {
        static CommServer server;
        static void Main()
        {
            CommServer server = new CommServer();
            server.Listen(8888);
            server.OnConnect += delegate ()
            {
                Console.WriteLine("An agent connects.");
            };
            server.OnReceive += delegate ()
            {
                byte[] data;
                IMsg msg;
                if (server.TryTake(out msg))
                {
                    MessageToServer mm = msg.Content as MessageToServer;
                    Console.WriteLine($"Receive a message from {mm.PlayerID}");
                    Console.WriteLine($"Message type::{mm.MessageType}");
                    if (mm.MessageType == MessageType.Send)
                    {
                        Console.WriteLine(mm.Message);
                    }

                }
                else
                {
                    Console.WriteLine("fail to dequeue");
                }
            };
            Console.WriteLine("============================");
            Console.ReadLine();

            {
                MessageToOneClient m = new MessageToOneClient();
                m.PlayerID = 0;
                m.TeamID = 0;
                m.MessageType = MessageType.ValidPlayer;
                m.Guid = 888;
                server.SendMessage(m);
                Console.WriteLine("已发送Validplayer");
            }

            Console.ReadLine();
            server.SendMessage(TestMessage(0,MessageType.StartGame));
            Console.WriteLine("已发送StartGame");
            Console.ReadLine();
            for(int i = 0; i < 100; i++)
            {
                server.SendMessage(TestMessage(i, MessageType.Gaming));
                Thread.Sleep(50);
            }
            Console.WriteLine("Gaming");
            Console.ReadLine();
            for (int i = 0; i < 100; i++)
            {
                server.SendMessage(TestMessage(i+100, MessageType.Gaming));
                Thread.Sleep(50);
            }
            server.SendMessage(TestMessage(666, MessageType.EndGame));
            Console.WriteLine("GameOver");
            Console.ReadLine();
            server.Dispose();
            server.Stop();
        }
        static private MessageToClient TestMessage(long guid, MessageType msgType)
        {

            var rows = 50;
            var cols = 50;

            //记录颜色信息，避免重复构造颜色信息
            Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneDimVec> msgCellColors = new Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneDimVec>();
            for (int x = 0; x < rows; ++x)
            {
                msgCellColors.Add(new MessageToClient.Types.OneDimVec());
                for (int y = 0; y < cols; ++y)
                {
                    msgCellColors[x].RowColors.Add(ColorType.Color2);
                }
            }


            //记录所有GUID信息
            Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneTeamGUIDs> playerGUIDs = new Google.Protobuf.Collections.RepeatedField<MessageToClient.Types.OneTeamGUIDs>();
            for (int x = 0; x < 2; ++x)
            {
                playerGUIDs.Add(new MessageToClient.Types.OneTeamGUIDs());
                for (int y = 0; y < 4; ++y)
                {
                    playerGUIDs[x].TeammateGUIDs.Add(x * 100 + y);
                }
            }

            MessageToClient msg = new MessageToClient();
            msg.PlayerID = 0;
            msg.TeamID = 0;
            msg.MessageType = msgType;
            msg.SelfInfo = new GameObjInfo();
            msg.SelfInfo.Ap = 100;
            msg.SelfInfo.BulletNum = 12;
            msg.SelfInfo.BulletType = BulletType.Bullet0;
            msg.SelfInfo.CanMove = true;
            msg.SelfInfo.CD = 5;
            msg.SelfInfo.FacingDirection = 0.0;
            msg.SelfInfo.GameObjType = GameObjType.Character;
            msg.SelfInfo.Hp = 1000;
            msg.SelfInfo.Guid = guid;
            msg.SelfInfo.IsDying = false;
            msg.SelfInfo.IsMoving = true;
            msg.SelfInfo.JobType = JobType.Job0;
            msg.SelfInfo.LifeNum = 10;
            msg.SelfInfo.MaxBulletNum = 50;
            msg.SelfInfo.MaxHp = 2500;
            msg.SelfInfo.MoveSpeed = 10;
            msg.SelfInfo.PropType = PropType.Null;
            msg.SelfInfo.Radius = 250;
            msg.SelfInfo.ShapeType = ShapeType.Circle;
            msg.SelfInfo.TeamID = 0;
            msg.SelfInfo.X = 5000;
            msg.SelfInfo.Y = 5000;
            for (int k = 0; k < 2; ++k)
            {
                msg.PlayerGUIDs.Add(playerGUIDs[k]);
            }
            msg.SelfTeamColor = ColorType.Color3;
            for (int x = 0; x < rows; ++x)
            {
                msg.CellColors.Add(msgCellColors[x]);
            }
            return msg;
        }
        
    }
}

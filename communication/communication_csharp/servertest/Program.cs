using System;
using System.Linq;
using Communication.CommServer;
using Communication.Proto;
using Google.Protobuf;
namespace servertest
{
    class Test
    {
        static void Main()
        {
            CommServer server = new CommServer();
            server.Listen(8888);
            MapColor mapColor;
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
                    Console.WriteLine($"Receive a message");
                    Message2Server mm = msg.Content as Message2Server;
                    Console.WriteLine($"Message type::{msg.MessageType}");
                    Console.WriteLine($"Info:{mm.JobType}");

                }
                else
                {
                    Console.WriteLine("fail to dequeue");
                }
            };
            while (true)
            {
                Message2Client mm = new Message2Client();
                mm.MapColors.Add(new Communication.Proto.MapColor
                {X=100,
                Y=200,
                TeamId=123
                }
                    );
                server.SendMessage(mm, MessageType.Message2Client);
            }

            Console.ReadLine();
            server.Dispose();
            server.Stop();
        }
    }
}

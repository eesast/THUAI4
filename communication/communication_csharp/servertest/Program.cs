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
                    Console.WriteLine($"Message type::{msg.PacketType}");
                    Console.WriteLine($"Info:{mm.JobType}");

                }
                else
                {
                    Console.WriteLine("fail to dequeue");
                }
            };
            Console.ReadLine();

            MessageToOneClient m = new MessageToOneClient();
            m.PlayerID = 1;
            m.TeamID = 123;
            m.MessageType = MessageType.AddPlayer;
            m.Guid = 888;
            server.SendMessage(m);


            Console.ReadLine();
            server.Dispose();
            server.Stop();
        }
    }
}

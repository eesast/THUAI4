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
                    Message2Server mm = msg.Content as Message2Server;
                    Console.WriteLine($"Receive a message from {mm.PlayerId}");
                    Console.WriteLine($"Message type::{msg.MessageType}");
                    Console.WriteLine($"Info:{mm.JobType}");

                }
                else
                {
                    Console.WriteLine("fail to dequeue");
                }
            };
            Console.ReadLine();

            Message2One m = new Message2One();
            m.PlayerId = 1;
            m.Message = "hello world!";
            server.SendMessage(m, MessageType.Message2One);


            Console.ReadLine();
            server.Dispose();
            server.Stop();
        }
    }
}

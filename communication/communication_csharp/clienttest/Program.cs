using System;
using Communication.CSharpClient;
using Communication.Proto;
using Google.Protobuf;

namespace clienttest
{
    class Test
    {
        static void Main(string[] args)
        {
            CSharpClient client = new CSharpClient();
            client.OnReceive += delegate (IMsg msg)
            {
                Message2Server mm = msg.Content as Message2Server;
                Console.WriteLine($"Message type::{msg.MessageType}");
                Console.WriteLine($"Info:{mm.JobType}");
            };
            Console.WriteLine("Connecting......");
            if (client.Connect("127.0.0.1", 7777))
            {
                Console.WriteLine("成功连接Agent.");
            }
            else
            {
                Console.WriteLine("连接Agent失败.");
            }
            string sendstr;
            byte[] bytes;
            while (true)
            {
                sendstr = Console.ReadLine();
                if (string.IsNullOrEmpty(sendstr)) break;
                Message2Server mm = new Message2Server();
                mm.JobType = Cummunication.Proto.JobType.Job1;
                client.SendMessage(mm, MessageType.Message2Server);
            }
            client.Stop();
            client.Dispose();
        }
    }
}

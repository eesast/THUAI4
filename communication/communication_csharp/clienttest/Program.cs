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
            Console.WriteLine($"My id is {int.Parse(args[1])}");
            CSharpClient client = new CSharpClient();
            client.OnReceive += delegate (IMsg msg)
            {
                Message2One mm = msg.Content as Message2One;
                Console.WriteLine($"Message type::{msg.MessageType}");
                Console.WriteLine("Message:" + mm.Message);
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
            

            Message2Server mm = new Message2Server();
            mm.PlayerId = int.Parse(args[1]);
            mm.JobType = Communication.Proto.JobType.Job1;
            client.SendMessage(mm, MessageType.Message2Server);

            Console.ReadLine();
            client.Stop();
            client.Dispose();
        }
    }
}

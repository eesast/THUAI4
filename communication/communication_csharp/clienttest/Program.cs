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
            client.OnReceive += delegate ()
            {
                IMsg msg = client.Take();
                MessageToOneClient mm = msg.Content as MessageToOneClient;
                Console.WriteLine($"Message type::{msg.PacketType}");
                Console.WriteLine(mm);
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
            

            MessageToServer mm = new MessageToServer();
            mm.PlayerID = int.Parse(args[1]);
            mm.TeamID = int.Parse(args[0]);
            mm.JobType = Communication.Proto.JobType.Job1;
            client.SendMessage(mm);

            Console.ReadLine();
            client.Stop();
            client.Dispose();
        }
    }
}

using System;
using System.Linq;
using Communication.CommServer;
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
                if (server.TryTake(out data))
                {
                    string temp;
                    temp = System.Text.Encoding.Default.GetString(data);
                    Console.WriteLine($"Receive :" + temp);
                    char[] arr = temp.ToCharArray();
                    data = System.Text.Encoding.Default.GetBytes(string.Concat<char>(arr.Reverse<char>()));
                    server.Send(data);
                }
                else
                {
                    Console.WriteLine("fail to dequeue");
                }
            };
            Console.ReadLine();
            server.Dispose();
            server.Stop();
        }
    }
}

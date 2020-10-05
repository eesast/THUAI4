using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using HPSocket.Tcp;
namespace clitest
{
    class Program
    {
        private static TcpClient client;
        static void Main(string[] args)
        {
            client = new TcpPackClient();
            client.Connect("127.0.0.1", 7777);
            byte[] temp= Encoding.Default.GetBytes("hello world");
            client.Send(temp, 12);
            client.Stop();
            Console.ReadLine();
        }
    }
}

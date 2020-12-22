using System;
using System.Net;
using System.Threading;
using Microsoft.Extensions.CommandLineUtils;
using HPSocket;
using HPSocket.Tcp;
namespace Communication.Agent
{
    class Program
    {
        private static TcpPackServer server = new TcpPackServer();
        private static TcpPackClient client = new TcpPackClient();
        static int Main(string[] args)
        {
            var app = new CommandLineApplication();
            app.HelpOption("-h|--help");
            var server = app.Option("-s|--server", "game server endpoint.", CommandOptionType.SingleValue);
            var port = app.Option("-p|--port", "agent port, 7777 indefault", CommandOptionType.SingleValue);
            app.OnExecute(() =>
            {
                string endpoint = server.Value();
                IPEndPoint serverend;
                try
                {
                    serverend = IPEndPoint.Parse(endpoint);
                }
                catch (Exception e)
                {
                    Console.WriteLine("Parsing server endpoint  went wrong:" + e.Message);
                    return 0;
                }
                string tt = port.Value();
                ushort agentport;
                agentport = string.IsNullOrEmpty(tt) ? (ushort)7777 : ushort.Parse(tt);
                Console.WriteLine("Server endpoint:" + serverend.Address.ToString() + ":" + serverend.Port.ToString());
                Console.WriteLine($"Agent port:{agentport}");
                return MainInternal(serverend, agentport);
            });
            app.Execute(args);
            return 0;
        }
        private static int MainInternal(IPEndPoint serverend, ushort agentport)
        {
            server.OnReceive += delegate (IServer sender, IntPtr connId, byte[] bytes)
            {
                client.Send(bytes, bytes.Length);
                return HandleResult.Ok;
            };
            client.OnReceive += delegate (IClient sender, byte[] bytes)
            {
                foreach (IntPtr connId in server.GetAllConnectionIds())
                {
                    if (!server.Send(connId, bytes, bytes.Length))
                    {
                        Console.WriteLine($"向{connId}发送失败。");
                    }
                }
                return HandleResult.Ok;
            };
            Console.WriteLine("Connecting......");
            if (!client.Connect(serverend.Address.ToString(), (ushort)serverend.Port))
            {
                Console.WriteLine("Failed to connect with the game server.");

            }
            else
            {
                int i;
                for (i = 0; i < 300; i++)
                {
                    if (client.IsConnected)
                    {
                        Console.WriteLine("Connect with the game server successfully.");
                        break;
                    }
                    else
                    {
                        Thread.Sleep(100);
                    }
                }
                if (i == 300) Console.WriteLine("Failed to connect with the game server.");
            }
            server.Port = agentport;
            if (server.Start())
            {
                Console.WriteLine("Agent starts listening.");
            }
            else
            {
                Console.WriteLine("Agent starts unsuccessfully");
            }
            Thread.Sleep(int.MaxValue);
            server.Stop();
            server.Dispose();
            client.Stop();
            client.Dispose();
            return 0;
        }
    }
}

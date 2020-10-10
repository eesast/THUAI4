using System;
using System.Net;
using System.Threading;
using Microsoft.Extensions.CommandLineUtils;
using HPSocket;
using HPSocket.Tcp;
using System.Collections.Generic;
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
                string ep = server.Value();
                if (string.IsNullOrEmpty(ep))
                {
                    Console.WriteLine("Fatal error: please input server endpoint.");
                    return 0;
                }
                string[] temp = ep.Split(":");
                if (temp.Length != 2)
                {
                    Console.WriteLine("Wrong format of server endpoint. IP:port");
                    return 0;
                }
                IPEndPoint serverend = new IPEndPoint(IPAddress.Parse(temp[0]), ushort.Parse(temp[1]));
                string tt = port.Value();
                ushort agentport;
                agentport = string.IsNullOrEmpty(tt) ? (ushort)7777 : ushort.Parse(tt);
                Console.WriteLine("Server endpoint:"+serverend.Address.ToString()+":"+serverend.Port.ToString());
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
                List<IntPtr> temp = server.GetAllConnectionIds();
                foreach (IntPtr connId in temp)
                {
                    if (!server.Send(connId, bytes, bytes.Length))
                    {
                        Console.WriteLine($"向{connId}发送失败。");
                        //如果有处理，就在这里加
                        //Agent怎么处理这里呢……我不知道
                    }
                }
                return HandleResult.Ok;
            };
            Console.WriteLine("Connecting......");
            if(!client.Connect(serverend.Address.ToString(), (ushort)serverend.Port))
            {
                Console.WriteLine("Failed to connect with the game server.");
                
            }
            else
            {
                int i;
                for(i = 0; i < 300; i++)
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
                if(i==300) Console.WriteLine("Failed to connect with the game server.");
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

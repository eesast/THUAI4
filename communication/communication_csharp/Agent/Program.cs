using System;
using System.Net;
using System.Threading;
using Microsoft.Extensions.CommandLineUtils;
using HPSocket;
using HPSocket.Tcp;
using System.Collections.Concurrent;
using Communication.Proto;
using Google.Protobuf;
using System.IO;

namespace Communication.Agent
{
    class Program
    {
        private static readonly TcpPackServer server = new TcpPackServer();
        private static readonly TcpPackClient client = new TcpPackClient();
        private static readonly ConcurrentDictionary<int, IntPtr> dict = new ConcurrentDictionary<int, IntPtr>();

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
            dict.Clear();
            server.OnReceive += delegate (IServer sender, IntPtr connId, byte[] bytes)
            {
                if (!dict.Values.Contains(connId))
                {
                    Message message = new Message();
                    message.MergeFrom(bytes);//这里需不需要拷贝？
                    if (message.MessageType == MessageType.Message2Server)//是否有其他可能？如何处理？
                    {
                        Message2Server msg = message.Content as Message2Server;
                        if (dict.ContainsKey(msg.PlayerId))
                        {
                            Console.WriteLine($"More than one client claims to have the same ID {msg.PlayerId}.");
                            return HandleResult.Error;
                        }
                        dict.TryAdd(msg.PlayerId, connId);//不可能false
                    }
                }
                client.Send(bytes, bytes.Length);
                return HandleResult.Ok;
            };
            //去年是Client退出前还要专门发一个消息，今年不太清楚，暂且如此
            server.OnClose += delegate (IServer sender, IntPtr connId, SocketOperation socketOperation, int errorCode)
            {
                foreach (int id in dict.Keys)
                {
                    if (dict[id] == connId)
                    {
                        if (!dict.TryRemove(id, out IntPtr temp)) return HandleResult.Error;
                        Console.WriteLine($"Player {id} closed the connection.");
                        break;
                    }
                }
                return HandleResult.Ok;
            };
            client.OnReceive += delegate (IClient sender, byte[] bytes)
            {
                MemoryStream istream = new MemoryStream(bytes);
                BinaryReader br = new BinaryReader(istream);
                MessageType type = (MessageType)br.PeekChar();
                br.Close();//debug期间加的，也许有用也许没用
                istream.Dispose();
                if (type == MessageType.Message2One)
                {
                    Message m = new Message();
                    m.MergeFrom(bytes);
                    Message2One message = m.Content as Message2One;
                    if (!dict.ContainsKey(message.PlayerId))
                    {
                        Console.WriteLine($"Error: No such player corresponding to ID {message.PlayerId}");
                        return HandleResult.Error;
                    }
                    if (!server.Send(dict[message.PlayerId], bytes, bytes.Length))
                    {
                        Console.WriteLine($"向{dict[message.PlayerId]}发送失败。");
                    }
                    return HandleResult.Ok;
                }
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
            dict.Clear();
            server.Stop();
            server.Dispose();
            client.Stop();
            client.Dispose();
            return 0;
        }
    }
}

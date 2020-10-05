using System;
using System.Collections.Concurrent;
using System.IO;
using HPSocket;
using HPSocket.Tcp;
using System.Text;
namespace Communication.Components
{
    //internal delegate void OnReceiveCallback(Message message);
    internal delegate void OnAcceptCallback();
    internal delegate void OnClientQuit();
    //server组件
    internal sealed class Server : IDisposable
    {
        private ConcurrentDictionary<int, IntPtr> clientList;
        private readonly TcpServer server;
        private bool isListening;//或许叫full更贴切
        private object idLock = new object();
        public ushort Port
        {
            get => server.Port;
            set => server.Port = value;
        }
        public int Count => clientList.Count;
        public Server()
        {
            clientList = new ConcurrentDictionary<int, IntPtr>();
            server = new TcpPackServer();
            isListening = false;
            //client字典由accept和close维护，receive只处理数据
            server.OnAccept += delegate (IServer sender, IntPtr connid, IntPtr client)
            {
                lock (idLock)
                {
                    if (!isListening)
                    {
                        server.Disconnect(connid);
                        return HandleResult.Ok;
                    }
                    int id = -1;
                    for (int i = 0; ; i++)
                    {
                        if (!clientList.ContainsKey(i))
                        {
                            id = i;
                            break;
                        }
                    }
                    clientList.TryAdd(id, connid);
                    Console.WriteLine($"ServerSide: Allocate ID #{id}");
                    //此处应该触发一个event，做一些什么时候不再监听的判断（Agent与server各异）或向网站发送信息
                }
                return HandleResult.Ok;
            };
            server.OnClose += delegate (HPSocket.IServer sender, IntPtr connid, HPSocket.SocketOperation socketoperation, int errorcode)
            {
                int id = -1;
                foreach (int key in clientList.Keys)
                {
                    if (clientList[key] == connid)
                    {
                        id = key;
                        break;
                    }
                }
                IntPtr tmp;
                clientList.TryRemove(id, out tmp);//out也不懂
                Console.WriteLine($"ServerSide: ID #{id } has quited");
                server.Disconnect(tmp);
                Resume();//改为监听状态
                return HPSocket.HandleResult.Ok;
            };

        }
        public void Start()
        {
            server.Start();
            Console.WriteLine($"Server has started on 0.0.0.0:{server.Port}");
            isListening = true;
            Console.WriteLine("Server is listening");

        }
        public void Pause()
        {
            isListening = false;
            Console.WriteLine("Server has paused, stop listening");
        }
        public void Resume()
        {
            isListening = true;
            Console.WriteLine("Server resume to listen");
        }
        public void Stop()
        {
           // Constants.Debug("ServerSide: Stopping");
            MemoryStream ostream = new MemoryStream();
            BinaryWriter bw = new BinaryWriter(ostream);
            //bw.Write((int)PacketType.Disconnected);
            byte[] raw = ostream.ToArray();
            foreach (IntPtr client in clientList.Values)
                server.Send(client, raw, raw.Length);
            server.Stop();
        }
        public void Dispose()
        {
            server.Dispose();
            GC.SuppressFinalize(this);
        }
    }
}

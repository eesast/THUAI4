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
    //改变一下架构，连接用onaccept和onclose维护
    internal sealed class Server : IDisposable
    {
        private ConcurrentDictionary<int, IntPtr> clientList;
        private readonly TcpServer server;
        private bool isListening;
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
            server.OnReceive += delegate (IServer sender, IntPtr connid, byte[] data)
            {
                Console.WriteLine($"Receive from {connid}:" + Encoding.Default.GetString(data));
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

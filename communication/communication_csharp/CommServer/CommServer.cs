using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using HPSocket;
using HPSocket.Tcp;
namespace Communication.CommServer
{
    public delegate void OnReceiveCallback(IntPtr connId);
    public delegate void OnConnectCallback(IntPtr connId);
    public sealed class CommServer : IDisposable
    {
        private ConcurrentQueue<byte[]> queue;
        private TcpPackServer server;
        public event OnReceiveCallback OnReceive;
        public event OnConnectCallback OnConnect;
        public CommServer()
        {
            server = new TcpPackServer();
            queue = new ConcurrentQueue<byte[]>();
            server.OnAccept += delegate (IServer sender, IntPtr connId, IntPtr client)
            {
                OnConnect?.Invoke(connId);
                return HandleResult.Ok;
            };
            server.OnReceive += delegate (IServer sender, IntPtr connId, byte[] bytes)
            {
                queue.Enqueue(bytes);
                OnReceive?.Invoke(connId);
                return HandleResult.Ok;
            };
        }
        public void Listen(ushort port)
        {
            server.Port = port;
            server.Start();
            Console.WriteLine($"Start listening to port {server.Port}");
        }
        public void Send(byte[] bytes)
        {
            Console.WriteLine("Boardcast");
            List<IntPtr> temp = server.GetAllConnectionIds();
            foreach (IntPtr connId in temp)
            {
                server.Send(connId, bytes, bytes.Length);
            }
        }
        public bool Dequeue(out byte[] data)//就是套了个壳，呵呵哒
        {
            Console.WriteLine("Dequeueing");
            return queue.TryDequeue(out data);
        }
        public void Stop()
        {
            server.Stop();
        }
        public void Dispose()
        {
            server.Dispose();
            GC.SuppressFinalize(this);
        }

    }
    /*
     class Test
    {
        static void Main()
        {
            CommServer server = new CommServer();
            server.Listen(8888);
            server.OnConnect += delegate (IntPtr connId)
            {
                Console.WriteLine($"Client {connId} connects.");
            };
            server.OnReceive += delegate (IntPtr connId)
            {
                byte[] data;
                if (server.Dequeue(out data))
                {
                    string temp;
                    temp = System.Text.Encoding.Default.GetString(data);
                    Console.WriteLine($"Receive from {connId}:" + temp);
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
    */
}


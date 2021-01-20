using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using HPSocket;
using HPSocket.Tcp;
using Communication.Proto;
using Google.Protobuf;

namespace Communication.CommServer
{
    public delegate void OnReceiveCallback();
    public delegate void OnConnectCallback();
    public sealed class CommServer : IDisposable
    {
        private BlockingCollection<IMsg> queue;
        private TcpPackServer server;
        public event OnReceiveCallback OnReceive;
        public event OnConnectCallback OnConnect;
        public CommServer()
        {
            server = new TcpPackServer();
            queue = new BlockingCollection<IMsg>();
            server.OnAccept += delegate (IServer sender, IntPtr connId, IntPtr client)
            {
                OnConnect?.Invoke();
                return HandleResult.Ok;
            };
            server.OnReceive += delegate (IServer sender, IntPtr connId, byte[] bytes)
            {
                Message message = new Message();
                message.MergeFrom(bytes);
                try
                {
                    queue.Add(message);//理论上这里可能抛出异常ObjectDisposedException或InvalidOperationException
                }
                catch (Exception e)
                {
                    Console.WriteLine("Exception occured when adding an item to the queue:" + e.Message);
                }
                OnReceive?.Invoke();
                return HandleResult.Ok;
            };
        }
        public bool Listen(ushort port)
        {
            server.Port = port;
            bool temp = server.Start();
            if (temp) Console.WriteLine($"Start listening to port {server.Port}");
            else Console.WriteLine("Failed to start server.");
            return temp;

        }
        public void SendMessage(IMessage msg,MessageType messagetype)
        {
            Message message = new Message();
            message.Content = msg;
            message.MessageType = messagetype;
            byte[] bytes;
            message.WriteTo(out bytes);
            Send(bytes);
        }
        private void Send(byte[] bytes)
        {
            List<IntPtr> temp = server.GetAllConnectionIds();
            foreach (IntPtr connId in temp)
            {
                if (!server.Send(connId, bytes, bytes.Length))
                {
                    Console.WriteLine($"向{connId}发送失败。");
                    //如果有处理，就在这里加
                }
            }
        }
        public bool TryTake(out IMsg msg)//就是套了个壳
        {
            try
            {
                return queue.TryTake(out msg);//理论上这里可能抛出异常ObjectDisposedException或InvalidOperationException
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception occured when trying to take an item from the queue:" + e.Message);
                msg = null;
                return false;
            }
        }
        public IMsg Take()
        {
            try
            {
                return queue.Take();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception occured when taking an item from the queue:" + e.Message);
                return null;
            }
        }
        public bool Stop()
        {
            return server.Stop();
        }
        public void Dispose()
        {
            server.Dispose();
            queue.Dispose();
            GC.SuppressFinalize(this);
        }

    }
}


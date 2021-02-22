using System;
using System.Threading;
using System.Collections.Generic;
using HPSocket;
using HPSocket.Tcp;
using Communication.Proto;
using System.Collections.Concurrent;
using Google.Protobuf;

namespace Communication.CSharpClient
{
    public delegate void OnReceiveCallback();
    public class CSharpClient : IDisposable
    {
        private TcpPackClient client;
        private BlockingCollection<IMsg> queue;
        public event OnReceiveCallback OnReceive;
        private readonly int maxtimeout=30000;//ms，超时connect还没成功则认为连接失败
        public CSharpClient()
        {
            client = new TcpPackClient();
            queue = new BlockingCollection<IMsg>();
            client.OnReceive += delegate (IClient sender, byte[] bytes)
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
        public bool Connect(string IP, ushort port)
        {
            client.Address = IP;
            client.Port = port;
            if (!client.Connect()) return false;
            for(int i = 0; i < maxtimeout / 100; i++)
            {
                if (!client.IsConnected)
                {
                    Thread.Sleep(100);
                }
                else
                {
                    return true;
                }
                    
            }
            return false;
        }
        public void SendMessage(MessageToServer msg)
        {
            Message message = new Message();
            message.Content = msg;
            message.PacketType = PacketType.MessageToServer;
            byte[] bytes;
            message.WriteTo(out bytes);
            Send(bytes);
        }
        private void Send(byte[] bytes)
        {
            if(!client.Send(bytes, bytes.Length))
            {
                Console.WriteLine("发送失败。");
            }
        }
        public bool TryTake(out IMsg msg)
        {
            try
            {
                return queue.TryTake(out msg);
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
            return client.Stop();
        }
        public void Dispose()
        {
            client.Dispose();
            GC.SuppressFinalize(this);
        }

    }
}


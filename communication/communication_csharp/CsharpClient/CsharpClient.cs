using System;
using System.Threading;
using System.Collections.Generic;
using HPSocket;
using HPSocket.Tcp;
namespace Communication.CSharpClient
{
    public delegate void OnReceiveCallback(byte[] bytes);
    public class CSharpClient : IDisposable
    {
        private TcpPackClient client;
        public event OnReceiveCallback OnReceive;
        private readonly int maxtimeout = 30000;//ms，超时connect还没成功则认为连接失败
        public CSharpClient()
        {
            client = new TcpPackClient();
            client.OnReceive += delegate (IClient sender, byte[] bytes)
            {
                OnReceive?.Invoke(bytes);
                return HandleResult.Ok;
            };
        }
        public bool Connect(string IP, ushort port)
        {
            client.Address = IP;
            client.Port = port;
            if (!client.Connect()) return false;
            for (int i = 0; i < maxtimeout / 100; i++)
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
        public bool Send(byte[] bytes)
        {
            return client.Send(bytes, bytes.Length);
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


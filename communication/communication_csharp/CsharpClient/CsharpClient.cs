using System;
using System.Threading;
using System.Collections.Generic;
using HPSocket;
using HPSocket.Tcp;
namespace Communication.CsharpClient
{
    public delegate void OnReceiveCallback(byte[] bytes);
    public class Csharpclient : IDisposable
    {
        private TcpPackClient client;
        private bool connected = false;
        public event OnReceiveCallback OnReceive;

        public Csharpclient()
        {
            connected = false;
            client = new TcpPackClient();
            client.OnConnect += delegate (IClient sender)
            {
                connected = true;
                return HandleResult.Ok;
            };
            client.OnClose += delegate (IClient sender, SocketOperation socketOperation, int errorCode)
            {
                connected = false;
                return HandleResult.Ok;
            };
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
            while (!client.IsConnected) Thread.Sleep(100);
            return true;
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


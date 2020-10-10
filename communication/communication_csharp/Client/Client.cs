using System;
using System.Threading;
using System.Collections.Generic;
using HPSocket;
using HPSocket.Tcp;
namespace Communication.Client
{
    public delegate void OnReceiveCallback(byte[] bytes);
    class Csharpclient : IDisposable
    {
        private TcpPackClient client;
        public event OnReceiveCallback OnReceive;

        public Csharpclient()
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
            return client.Connect();
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
    /*class Test
    {
        static void Main(string[] args)
        {
            Csharpclient client = new Csharpclient();
            client.OnReceive += delegate (byte[] bytes)
            {
                string temp = System.Text.Encoding.Default.GetString(bytes);
                Console.WriteLine("Receive from server:" + temp);
            };
            client.Connect("127.0.0.1", 7777);
            string sendstr;
            byte[] bytes;
            while (true)
            {
                sendstr = Console.ReadLine();
                if (string.IsNullOrEmpty(sendstr)) break;
                bytes = System.Text.Encoding.Default.GetBytes(sendstr);
                client.Send(bytes);
            }
            client.Stop();
            client.Dispose();
        }
    }*/
}


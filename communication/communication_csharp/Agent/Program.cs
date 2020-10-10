using System;
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
            var port = app.Option("-p|--port", "agent port, 7777 in default", CommandOptionType.SingleValue);
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
                string tt = port.Value();
                return MainInternal(temp[0], ushort.Parse(temp[1]), string.IsNullOrEmpty(tt) ? (ushort)7777 : ushort.Parse(tt));
            });
            app.Execute(args);
            return 0;
        }
        private static int MainInternal(string sip, ushort sport, ushort aport)
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
                    server.Send(connId, bytes, bytes.Length);
                }
                return HandleResult.Ok;
            };
            client.Address = sip;
            client.Port = sport;
            client.Connect();
            while (!client.IsConnected) Thread.Sleep(100);
            server.Port = aport;
            server.Start();
            Thread.Sleep(int.MaxValue);
            server.Stop();
            server.Dispose();
            client.Stop();
            client.Dispose();
            return 0;
        }
    }
}

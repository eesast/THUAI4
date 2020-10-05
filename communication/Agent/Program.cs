using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using Microsoft.Extensions.CommandLineUtils;
using HPSocket.Tcp;
using Communication.Proto;
namespace Communication.Agent
{
    public class Program
    {
        private static TcpServer server;
        static void Main(string[] args)
        {
            var app = new CommandLineApplication();
            app.HelpOption("-h|--help");
            var server = app.Option("-s|--server", "game server endpoint IP:port", CommandOptionType.SingleValue);
            var port = app.Option("-p|--port", "agent port", CommandOptionType.SingleValue);
            var dlevel = app.Option("-d|--debuglevel", "debug level: 0 disabled( and default),1 normal, 2 verbose) ", CommandOptionType.SingleValue);
            app.OnExecute(()=>
            {
                Console.WriteLine("Server :"+server.Value());
                Console.WriteLine("Agent port:" + port.Value());
                Console.WriteLine("Debug level:" + dlevel.Value());
                int level = string.IsNullOrEmpty(dlevel.Value()) ? 0 : int.Parse(dlevel.Value());
                return MainInternal(ushort.Parse(port.Value()),level);
            });
            app.Execute(args);
        }
        private static int MainInternal(ushort port,int dlevel)
        {
            Debug.DebugLevel = dlevel;
            server = new TcpPackServer();
            server.Port = port;
            server.OnPrepareListen += delegate (HPSocket.IServer sender, IntPtr listens){
                Console.WriteLine("Listenning prepared.");
                return HPSocket.HandleResult.Ok;
            };
            server.OnAccept += delegate (HPSocket.IServer sender,IntPtr connid,IntPtr client)
              {
                  Console.WriteLine($"Accept.\tConnid:{connid}\tclient:{client}");
                  return HPSocket.HandleResult.Ok;
              };
            server.OnReceive += delegate (HPSocket.IServer sender, IntPtr connid, byte[] data)
             {
                 Console.WriteLine($"Receive from {connid}:" + Encoding.Default.GetString(data));
                 return HPSocket.HandleResult.Ok;
             };
            server.OnClose += delegate (HPSocket.IServer sender, IntPtr connid, HPSocket.SocketOperation socketoperation, int errorcode)
              {
                  Console.WriteLine($"Connection with {connid} is closed.\nsocketoperation:{socketoperation}\terrorcode:{errorcode}");
                 return HPSocket.HandleResult.Ok;
              };
            server.Start();
            Thread.Sleep(int.MaxValue);
            return 0;
        }
    }
}

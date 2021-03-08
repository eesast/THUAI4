using System;
using CommandLine;

namespace Logic.Server
{
    class Program
    {
        static int Main(string[] args)
        {
            ArgumentOptions? options = null;
            Parser.Default.ParseArguments<ArgumentOptions>(args).WithParsed(o => { options = o; });
            if (options == null)
			{
                Console.WriteLine("Argument parsing failed!");
                return 1;
			}

            Console.WriteLine("Server begin to run: " + options.ServerPort.ToString());

            Server server = new Server(options);
            
            return 0;
        }
    }
}

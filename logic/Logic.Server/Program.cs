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
                Console.WriteLine("Parse failed!");
                return 1;
			}

            Console.WriteLine(options.ServerPort);
            return 0;
        }
    }
}

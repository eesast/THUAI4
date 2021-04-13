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

            ServerBase server = null;

            if (options.PlayBack)
            {
                server = new PlayBackServer(options);

            }
            else
            {
                server = new GameServer(options);
            }

            Console.WriteLine($"Final score: ");
            for (int i = 0; i < server.TeamCount; ++i)
			{
                Console.WriteLine($"Team {i}: {server.GetTeamScore(i)}");
			}

            Console.WriteLine("Press any key to continue...");
            Console.ReadKey();
            
            return 0;
        }
    }
}

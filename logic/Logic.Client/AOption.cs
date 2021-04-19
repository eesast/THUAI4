using System;
using System.Collections.Generic;
using System.Text;
using CommandLine;

namespace Logic.Client
{
    class AOption
    {
        [Option('p', "port", Required = true, HelpText = "Server listening port")]
        public ushort ServerPort { get; set; } = 10086;

        [Option('t', "teamID", Required = true, HelpText = "Team ID")]
        public ushort teamID { get; set; } = 1;

        [Option('i', "playerID", Required = true, HelpText = "Player ID")]
        public ushort playerID { get; set; } = 1;

        [Option('j', "job", Required = true, HelpText = "choose your job")]
        public ushort job { get; set; } = 0;

        [Option('w', "watch", Required = false, HelpText = "watch or not")]
        public bool watch { get; set; } = false;
    }
}

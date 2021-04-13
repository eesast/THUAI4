using CommandLine;

namespace Logic.Server
{
	class ArgumentOptions
	{
		[Option('k', "token", Required = false, HelpText = "Web API Token")]
		public string Token { get; set; } = "";

		[Option('p', "port", Required = true, HelpText = "Server listening port")]
		public ushort ServerPort { get; set; } = 10086;

		[Option('t', "teamCount", Required = false, HelpText = "The number of teams, 1 by defualt")]
		public ushort TeamCount { get; set; } = 1;

		[Option('c', "playerCount", Required = false, HelpText = "The number of players per team, 1 by default")]
		public ushort PlayerCountPerTeam { get; set; } = 1;

		[Option('g', "gameTimeInSecond", Required = false, HelpText = "The time of the game in second, 10 minutes by default")]
		public uint GameTimeInSecond { get; set; } = 10 * 60;

		[Option('f', "fileName", Required = false, HelpText = "The file to store playback file or to read file.")]
		public string FileName { get; set; } = "";

		[Option('b', "playBack", Required = false, HelpText = "Whether open the server in a playback mode.")]
		public bool PlayBack { get; set; } = false;
	}
}

namespace Logic.Server
{
	static class ServerFactory
	{
		public static ServerBase GetServer(ArgumentOptions options)
		{
			ServerBase server;
			if (options.PlayBack)
			{
				server = new PlayBackServer(options);

			}
			else if (options.RequestOnly)
			{
				server = new RequestOnlyServer(options);
			}
			else
			{
				server = options.FinalGame ? new FinalGameServer(options)
					: options.CheatMode ? new CheatServer(options)
					: new GameServer(options);
			}
			return server;
		}
	}
}

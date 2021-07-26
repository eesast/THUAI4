using System;

namespace PlayBackPlayer
{
	class Program
	{
		static void Main(string[] args)
		{
			PlayBackPlayerDll.PlayBackPlayerDll.Main(args.Length >= 1 ? args[0] : null);
		}
	}
}

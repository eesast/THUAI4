using System;
using THUnity2D;
using Gaming;
using GameEngine;
using System.Threading;
using System.Runtime.InteropServices;

namespace test
{
	class Win32Api
	{
		[DllImport("user32.dll", CallingConvention = CallingConvention.StdCall)]
		public static extern Int16 GetKeyState(Int32 nVirtKey);
	}
	class Program
	{
		static void Main(string[] args)
		{
			Game game = new Game(MapInfo.defaultMap, 1);
			long tmpID;
			long[] player2ID = new long[2];

			XYPosition player1Pos = Map.Constant.CellToGrid(1, 1);
			XYPosition player2Pos = Map.Constant.CellToGrid(2, 2);

			if ((tmpID = game.AddPlayer(new Game.PlayerInitInfo(0u, (JobType)0, 0))) == GameObject.invalidID)
			{
				Console.WriteLine("Add player failed!");
			}
			else
			{
				player2ID[0] = (tmpID);
			}

			if (((tmpID) = game.AddPlayer(new Game.PlayerInitInfo(1u, (JobType)0, 0))) == GameObject.invalidID)
			{
				Console.WriteLine("Add player failed!");
			}
			else player2ID[1] = tmpID;

			new Thread
				(
					() =>
					{
						game.StartGame(5 * 60 * 1000);
					}
				)
			{ IsBackground = true }.Start();

			new Thread
				(
					() =>
					{
						double[] direct = new double[16];
						int[] time = new int[16];

						const int WKey = 0x1;
						const int AKey = 0x2;
						const int SKey = 0x4;
						const int DKey = 0x8;

						for (int i = 1; i < time.Length; ++i)
						{
							time[i] = 500;
						}

						direct[WKey] = Math.PI / 2;
						direct[AKey] = Math.PI;
						direct[SKey] = -Math.PI / 2;
						direct[DKey] = 0.0;
						direct[WKey | AKey] = Math.PI / 4 * 3;
						direct[WKey | DKey] = Math.PI / 4;
						direct[SKey | AKey] = Math.PI / 4 * 5;
						direct[SKey | DKey] = Math.PI / 4 * 7;

						while (true)
						{
							Thread.Sleep(500);
							int key = 0;
							bool WPress = Win32Api.GetKeyState((Int32)ConsoleKey.UpArrow) < 0,
								APress = Win32Api.GetKeyState((Int32)ConsoleKey.LeftArrow) < 0,
								SPress = Win32Api.GetKeyState((Int32)ConsoleKey.DownArrow) < 0,
								DPress = Win32Api.GetKeyState((Int32)ConsoleKey.RightArrow) < 0;
							if (WPress) key |= WKey;
							if (APress) key |= AKey;
							if (SPress) key |= SKey;
							if (DPress) key |= DKey;
							if (key != 0)
							{
								game.MovePlayer((long)player2ID[1], time[key], direct[key]);
							}
						}
					}
				)
			{ IsBackground = true }.Start();

			double[] direct = new double[16];
			int[] time = new int[16];

			const int WKey = 0x1;
			const int AKey = 0x2;
			const int SKey = 0x4;
			const int DKey = 0x8;
		   
			for (int i = 1; i < time.Length; ++i)
			{
				time[i] = 500;
			}

			direct[WKey] = Math.PI / 2;
			direct[AKey] = Math.PI;
			direct[SKey] = -Math.PI / 2;
			direct[DKey] = 0.0;
			direct[WKey | AKey] = Math.PI / 4 * 3;
			direct[WKey | DKey] = Math.PI / 4;
			direct[SKey | AKey] = Math.PI / 4 * 5;
			direct[SKey | DKey] = Math.PI / 4 * 7;

			while (true)
			{
				Thread.Sleep(500);
				int key = 0;
				bool WPress = Win32Api.GetKeyState((Int32)ConsoleKey.W) < 0,
					APress = Win32Api.GetKeyState((Int32)ConsoleKey.A) < 0,
					SPress = Win32Api.GetKeyState((Int32)ConsoleKey.S) < 0,
					DPress = Win32Api.GetKeyState((Int32)ConsoleKey.D) < 0;
				if (WPress) key |= WKey;
				if (APress) key |= AKey;
				if (SPress) key |= SKey;
				if (DPress) key |= DKey;
				if (key != 0)
				{
					game.MovePlayer((long)player2ID[0], time[key], direct[key]);
				}

				if (Win32Api.GetKeyState((Int32)ConsoleKey.J) < 0)
				{
					game.Attack((long)player2ID[0], 10000000, Math.PI);
				}
			}
		}
	}
}

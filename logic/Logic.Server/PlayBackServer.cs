using Communication.Proto;
using playback;
using System;
using System.Threading;
using System.Threading.Tasks;
using Timothy.FrameRateTask;

namespace Logic.Server
{
	class PlayBackServer : ServerBase
	{
		private int[] teamScore;

		public override int TeamCount { get => teamScore.Length; }
		public override bool IsWebCompetition => false;

		public PlayBackServer(ArgumentOptions options) : base(options)
		{
			try
			{
				int timeInterval = GameServer.SendMessageToClientIntervalInMilliseconds;
				if (options.PlayBackSpeed != 1.0)
				{
					options.PlayBackSpeed = Math.Max(0.25, Math.Min(4.0, options.PlayBackSpeed));
					timeInterval = (int)Math.Round(timeInterval / options.PlayBackSpeed);
				}
				using (MessageReader mr = new MessageReader(options.FileName))
				{
					teamScore = new int[mr.teamCount];
					int infoNo = 0;
					var frt = new FrameRateTaskExecutor<int>
						(
						loopCondition: () => true,
						loopToDo: () =>
						{
							MessageToClient? msg = null;
							for (int i = 0; i < mr.teamCount; ++i)
							{
								for (int j = 0; j < mr.playerCount; ++j)
								{
									msg = mr.ReadOne();
									if (msg == null)
									{
										Console.WriteLine("The game doesn't come to an end because of timing up!");
										return false;
									}
									serverCommunicator.SendMessage(msg);
									Console.WriteLine($"Seccessfully sent a message. Message number: {infoNo}.");
									if (msg != null)
									{
										teamScore[i] = msg.TeamScore;
									}
								}
							}
							++infoNo;
							if (msg == null)
							{
								Console.WriteLine("No game information in this file!");
								return false;
							}
							if (msg.MessageType == MessageType.EndGame)
							{
								Console.WriteLine("Game over normally!");
								return false;
							}
							return true;
						},
						timeInterval: timeInterval,
						finallyReturn: () => 0
						)
					{ AllowTimeExceed = true, MaxTolerantTimeExceedCount = 5 };

					Console.WriteLine("The server is well prepared! Please MAKE SURE that you have opened all the clients to watch the game!");
					Console.WriteLine("If ALL clients have opened, press any key to start.");
					Console.ReadKey();

					Task.Run
						(
							() =>
							{
								while (!frt.Finished)
								{
									Console.WriteLine($"Send message to clients frame rate: {frt.FrameRate}");
									Thread.Sleep(1000);
								}
							}
						);

					frt.Start();
				}
			}
			finally
			{
				if (teamScore == null)
				{
					teamScore = new int[1];
				}
			}
		}
		protected override void OnReceive(MessageToServer msg) { }
		public override int GetTeamScore(long teamID)
		{
			return teamScore[(int)teamID];
		}
	}
}

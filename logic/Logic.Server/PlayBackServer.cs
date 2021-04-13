using Communication.Proto;
using System;
using System.Collections.Generic;
using System.Text;
using playback;
using Timothy.FrameRateTask;
using System.Threading;
using System.Threading.Tasks;

namespace Logic.Server
{
	class PlayBackServer : ServerBase
	{
		private int[] teamScore;

		public override int TeamCount { get => teamScore.Length; }

		public PlayBackServer(ArgumentOptions options) : base(options)
		{
			try
			{
				using (MessageReader mr = new MessageReader(options.FileName))
				{
					teamScore = new int[mr.teamCount];
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
									if (msg == null) return false;
									serverCommunicator.SendMessage(msg);
									Console.WriteLine("Seccessfully sent a message.");
									if (msg != null)
									{
										teamScore[i] = msg.TeamScore;
									}
								}
							}
							if (msg.MessageType == MessageType.EndGame) return false;
							return true;
						},
						GameServer.SendMessageToClientIntervalInMilliseconds,
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
			catch (Exception e)
			{
				Console.WriteLine(e);
				return;
			}
		}
		protected override void OnReceive(MessageToServer msg) { }
		public override int GetTeamScore(long teamID)
		{
			return teamScore[(int)teamID];
		}
	}
}

using Communication.Proto;

namespace Logic.Server
{
	/// <summary>
	/// 作弊模式 Server
	/// </summary>
	class CheatServer : GameServer
	{
		private string cheatCode = "Make EE hard again!";

		public CheatServer(ArgumentOptions options) : base(options)
		{

		}

		protected override void OnReceive(MessageToServer msg)
		{
			if (msg.MessageType == MessageType.Send && msg.Message == cheatCode)
			{
				game.Cheat(communicationToGameID[msg.TeamID, msg.PlayerID]);
			}
			base.OnReceive(msg);
		}
	}
}

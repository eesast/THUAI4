using Communication.Proto;
using Newtonsoft.Json.Linq;

namespace Logic.Server
{
	class RequestOnlyServer : ServerBase
	{
		private int[] teamScore;
		public override int TeamCount { get => teamScore.Length; }
		protected override void OnReceive(MessageToServer msg) { }
		public override int GetTeamScore(long teamID)
		{
			return teamScore[(int)teamID];
		}
		public override bool IsWebCompetition => true;
		private HttpSender httpSender;
		public RequestOnlyServer(ArgumentOptions options) : base(options)
		{
			teamScore = new int[options.TeamCount];
			httpSender = new HttpSender(options.Url, options.Token, "PUT");

			var scores = new JObject[options.TeamCount];
			for (ushort i = 0; i < options.TeamCount; ++i)
			{
				scores[i] = new JObject { ["team_id"] = i.ToString(), ["score"] = GetTeamScore(i) };
			}
			httpSender?.SendHttpRequest
				(
					new JObject
					{
						["result"] = new JArray(scores)
					}
				);
		}
	}
}

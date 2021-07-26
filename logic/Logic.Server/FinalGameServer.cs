using System.IO;

namespace Logic.Server
{
	class FinalGameServer : GameServer
	{
		public override bool ForManualOperation => false;

		private string resultFileName = "/usr/local/mnt/score";

		public FinalGameServer(ArgumentOptions options) : base(options)
		{

			// 原定通过环境变量获取 ID，现取消此设定

			// string[] webTeamIDs = new string[options.TeamCount];
			//for (int i = 0; i < options.TeamCount; ++i)
			//{
			//	var s = Environment.GetEnvironmentVariable("team" + i.ToString() + "ID");      // 通过环境变量获取队伍 ID
			//	webTeamIDs[i] = s ?? "";
			//}

			//resultFileName = webTeamIDs.Length == 0 ? "" : webTeamIDs[0];
			//for (int i = 1; i < webTeamIDs.Length; ++i)
			//{
			//	resultFileName += ".vs." + webTeamIDs[i];
			//}
			//resultFileName += ".res";
		}

		protected override void SendGameResult()		// 决赛时 server 把比赛结果写入文件
		{
			using (StreamWriter sw = new StreamWriter(resultFileName, false))
			{
				for (int i = 0; i < TeamCount; ++i)
				{
					sw.Write(GetTeamScore(i).ToString() + ',');
				}
				sw.WriteLine();
				sw.Flush();
			}
		}
	}
}

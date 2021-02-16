using System;
using System.Collections.Generic;
using System.Text;
using THUnity2D;

namespace Logic.Server
{
	class Server
	{
		private Map game;
		private ArgumentOptions options;

		Tuple<long, Map.PlayerInitInfo>[,] communicationToGameID;		//把通信所用ID映射到游戏ID和初始化信息，[i, j]代表第 i 个队伍的第 j 个玩家的 id 和初始化信息
		
		public Server(ArgumentOptions options)
		{
			//队伍数量在 1~4 之间，总人数不超过 8
			if (options.TeamCount > 4) options.TeamCount = 4;
			if (options.TeamCount < 1) options.TeamCount = 1;
			if (options.PlayerCountPerTeam * options.TeamCount > 8) options.PlayerCountPerTeam = (ushort)(8 / options.TeamCount);
			if (options.PlayerCountPerTeam < 1) options.PlayerCountPerTeam = 1;

			this.options = options;
			game = new Map(MapInfo.map, options.TeamCount);
			communicationToGameID = new Tuple<long, Map.PlayerInitInfo>[options.TeamCount, options.PlayerCountPerTeam];
			for (int i = 0; i < communicationToGameID.GetLength(0); ++i)
			{
				for (int j = 0; j < communicationToGameID.GetLength(1); ++j)
				{
					communicationToGameID[i, j] = new Tuple<long, Map.PlayerInitInfo>(GameObject.invalidID, new Map.PlayerInitInfo());
				}
			}
		}
	}
}

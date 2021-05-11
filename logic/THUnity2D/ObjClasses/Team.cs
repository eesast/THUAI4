using System.Collections.Generic;

//队伍

namespace THUnity2D.ObjClasses
{
	// 队伍类。此类并发不安全，须谨慎使用（需要并发安全须额外加锁）
	public class Team
	{
		private static long currentMaxTeamID = 0;
		public static long CurrentMaxTeamID => currentMaxTeamID;
		private readonly long teamID;
		public long TeamID => teamID;
		public const long invalidTeamID = long.MaxValue;
		public const long noneTeamID = long.MinValue;
		public int Score
		{
			get
			{
				int score = 0;
				foreach (var player in playerList)
				{
					score += player.Score;
				}
				return score;
			}
		}

		private List<Character> playerList;

		public Character? GetPlayer(long playerID)
		{
			foreach (Character player in playerList)
			{
				if (player.ID == playerID)
				{
					return player;
				}
			}
			return null;
		}

		//并发不安全，请谨慎使用此方法，目前此方法仅由 Map 的 GetGameObject 调用，其余地方不要调用
		public List<Character> GetPlayerListForUnsafe()
		{
			return playerList;
		}

		public void AddPlayer(Character player)
		{
			playerList.Add(player);
		}
		public void OutPlayer(long playerID)
		{
			int i;
			for (i = 0; i < playerList.Count; ++i)
			{
				if (playerList[i].ID == playerID) break;
			}
			playerList.RemoveAt(i);
		}
		public void ClearPlayer()
		{
			playerList = new List<Character>();
		}
		public long[] GetPlayerIDs()
		{
			long[] playerIDs = new long[playerList.Count];
			int num = 0;
			foreach (Character player in playerList)
			{
				playerIDs[num++] = player.ID;
			}
			return playerIDs;
		}
		public static bool teamExists(long findTeamID)
		{
			return findTeamID < currentMaxTeamID;
		}
		public Team()
		{
			teamID = currentMaxTeamID++;
			playerList = new List<Character>();
		}
	}
}

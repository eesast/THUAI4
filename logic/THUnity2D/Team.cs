using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

//队伍

namespace THUnity2D
{
	public class Team
	{
		private static long currentMaxTeamID = 0;
		public static long CurrentMaxTeamID
		{
			get => currentMaxTeamID;
		}
		private readonly long teamID;
		public long TeamID { get => teamID; }
		public const long invalidTeamID = long.MaxValue;
		public const long noneTeamID = long.MinValue;
		private ArrayList playerList;
		public int Score
		{
			get
			{
				int score = 0;
				foreach (var player in playerList)
				{
					score += ((Character)player).Score;
				}
				return score;
			}
		}

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

		public void AddPlayer(Character player)
		{
			playerList.Add(player);
		}
		public void OutPlayer(long playerID)
		{
			int i;
			for (i = 0; i < playerList.Count; ++i)
			{
				if (((Character)playerList[i]).ID == playerID) break;
			}
			playerList.RemoveAt(i);
		}
		public void ClearPlayer()
		{
			playerList = ArrayList.Synchronized(new ArrayList());
		}
		public static bool teamExists(long findTeamID)
		{
			return findTeamID < currentMaxTeamID;
		}
		public Team()
		{
			teamID = currentMaxTeamID++;
			playerList = ArrayList.Synchronized(new ArrayList());
		}
	}
}

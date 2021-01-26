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
		public bool GetPlayerScore(long playerID, out int score)
		{
			foreach (Character player in playerList)
			{
				if (player.ID == playerID)
				{
					score = player.Score;
					return true;
				}
			}
			score = 0;
			return false;
		}
		public bool GetPlayerHP(long playerID, out int hp)
		{
			foreach (Character player in playerList)
			{
				if (player.ID == playerID)
				{
					hp = player.HP;
					return true;
				}
			}
			hp = 0;
			return false;
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

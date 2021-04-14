using GameEngine;
using System;
using THUnity2D;

namespace Gaming
{
	public partial class Game
	{
		private MoveManager moveManager;
		private class MoveManager
		{

			//人物移动
			public void MovePlayer(Character playerToMove, int moveTimeInMilliseconds, double moveDirection)
			{
				moveEngine.MoveObj(playerToMove, moveTimeInMilliseconds, moveDirection);
			}


			private void ActivateMine(Character player, Mine mine)
			{
				gameMap.ObjListLock.EnterWriteLock();
				try { gameMap.ObjList.Remove(mine); }
				catch { }
				finally { gameMap.ObjListLock.ExitWriteLock(); }

				switch (mine.GetPropType())
				{
					case PropType.Dirt:
						player.AddMoveSpeed(Map.Constant.dirtMoveSpeedDebuff, Map.Constant.buffPropTime);
						break;
					case PropType.Attenuator:
						player.AddAP(Map.Constant.attenuatorAtkDebuff, Map.Constant.buffPropTime);
						break;
					case PropType.Divider:
						player.ChangeCD(Map.Constant.dividerCdDiscount, Map.Constant.buffPropTime);
						break;
				}
			}


			private Map gameMap;
			private MoveEngine moveEngine;
			public MoveManager(Map gameMap)
			{
				this.gameMap = gameMap;
				this.moveEngine = new MoveEngine
				(
					gameMap: gameMap,
					OnCollision: (obj, collisionObj, moveVec) =>
					{
						if (collisionObj is Mine)
						{
							ActivateMine((Character)obj, (Mine)collisionObj);
							return MoveEngine.AfterCollision.ContinueCheck;
						}
						return MoveEngine.AfterCollision.MoveMax;
					},
					EndMove: obj =>
					{
						GameObject.Debug(obj, " end move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64);
					},
					IgnoreCollision: (obj, collisionObj) =>
					{
						if (collisionObj is BirthPoint)         // 自己的出生点可以忽略碰撞
						{
							if (object.ReferenceEquals(((BirthPoint)collisionObj).Parent, obj)) return true;
							return false;
						}
						else if (collisionObj is Mine)          // 自己队的炸弹忽略碰撞
						{
							if (((Mine)collisionObj).Parent.TeamID == ((Character)obj).TeamID) return true;
						}
						return false;
					}
				);

			}
		}
	}
}

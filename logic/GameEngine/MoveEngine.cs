using System;
using System.Threading.Tasks;
using THUnity2D;
using Timothy.FrameRateTask;

namespace GameEngine
{
	public class MoveEngine
	{

		/// <summary>
		/// 碰撞结束后要做的事情
		/// </summary>
		public enum AfterCollision
		{
			ContinueCheck = 0,      // 碰撞后继续检查其他碰撞
			MoveMax = 1,            // 行走最远距离
			Destroyed = 2           // 物体已经毁坏
		}

		/// <summary>
		/// 在无碰撞的前提下行走最远的距离
		/// </summary>
		/// <param name="obj">移动物体，默认obj.Rigid为true</param>
		/// <param name="moveVec">移动的位移向量</param>
		private void MoveMax(GameObject obj,  Vector moveVec)
		{

			/*由于四周是墙，所以人物永远不可能与越界方块碰撞*/

			XYPosition nextPos = obj.Position + Vector.Vector2XY(moveVec);

			uint maxLen = collisionChecker.FindMax(obj, nextPos, moveVec);

			maxLen = (uint)Math.Min(maxLen, (obj.MoveSpeed / Map.Constant.numOfStepPerSecond));
			obj.Move(new Vector(moveVec.angle, maxLen));
		}

		/// <summary>
		/// 移动物体
		/// </summary>
		/// <param name="obj">要移动的物体</param>
		/// <param name="moveTime">移动的时间，毫秒</param>
		/// <param name="moveDirection">移动的方向，弧度</param>
		public void MoveObj(GameObject obj, int moveTime, double moveDirection)
		{
			Task.Run
			(
				() =>
				{
					lock (obj.moveLock)
					{
						if (!obj.IsAvailable) return;
						obj.IsMoving = true;     //开始移动
					}

					GameObject.Debug(obj, " begin to move at " + obj.Position.ToString() + " At time: " + Environment.TickCount64.ToString());
					double deltaLen = 0.0;      //储存行走的误差
					Vector moveVec = new Vector(moveDirection, 0.0);
					//先转向
					if (gameMap.Timer.IsGaming && obj.CanMove) deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));     //先转向
					GameObject? collisionObj = null;

					bool isDestroyed = false;
					new FrameRateTaskExecutor<int>
					(
						() => gameMap.Timer.IsGaming && obj.CanMove && !obj.IsResetting,
						() =>
						{
							moveVec.length = obj.MoveSpeed / Map.Constant.numOfStepPerSecond + deltaLen;
							deltaLen = 0;

							//越界情况处理：如果越界，则与越界方块碰撞

							do
							{
							Check:
								collisionObj = collisionChecker.CheckCollision(obj, moveVec);
								if (collisionObj == null) break;

								switch (OnCollision(obj, collisionObj, moveVec))
								{
									case AfterCollision.ContinueCheck: goto Check;
									case AfterCollision.Destroyed:
										GameObject.Debug(obj, " collide with " + collisionObj.ToString() + " and has been removed from the game.");
										isDestroyed = true;
										return false;
									case AfterCollision.MoveMax:
										MoveMax(obj, moveVec);
										moveVec.length = 0;
										break;
								}
							} while (false);

							deltaLen += moveVec.length - Math.Sqrt(obj.Move(moveVec));

							return true;
						},
						1000 / Map.Constant.numOfStepPerSecond,
						() =>
						{
							if (!isDestroyed)
							{
								moveVec.length = deltaLen;
								if ((collisionObj = collisionChecker.CheckCollision(obj, moveVec)) == null)
								{
									obj.Move(moveVec);
								}
								else
								{
									OnCollision(obj, collisionObj, moveVec);
								}
								obj.IsMoving = false;        //结束移动
								EndMove(obj);
							}
							return 0;
						},
						maxTotalDuration: moveTime
					)
					{
						AllowTimeExceed = true,
						MaxTolerantTimeExceedCount = ulong.MaxValue,
						TimeExceedAction = b =>
						{
							Console.WriteLine("The computer runs so slow that the player cannot finish moving during this time!!!!!!");
						}
					}.Start();
				}
			);
		}

		private Map gameMap;
		private Action<GameObject> EndMove;
		private CollisionChecker collisionChecker;
		private Func<GameObject, GameObject, Vector, AfterCollision> OnCollision;


		/// <summary>
		/// Constrctor
		/// </summary>
		/// <param name="gameMap">游戏地图</param>
		/// <param name="OnCollision">发生碰撞时要做的事情，第一个参数为移动的物体，第二个参数为撞到的物体，第三个参数为移动的位移向量，返回值见AfterCollision的定义</param>
		/// <param name="EndMove">结束碰撞时要做的事情</param>
		/// <param name="IgnoreCollision">是否忽略本次碰撞。第一个参数为移动的物体，第二个参数为撞到的物体。如果忽略本次碰撞，返回true；否则返回false</param>
		public MoveEngine
			(
				Map gameMap,
				Func<GameObject, GameObject, Vector, AfterCollision> OnCollision,
				Action<GameObject> EndMove,
				Func<GameObject, GameObject, bool> IgnoreCollision
			)
		{
			this.gameMap = gameMap;
			this.EndMove = EndMove;
			this.OnCollision = OnCollision;
			this.collisionChecker = new CollisionChecker(gameMap, IgnoreCollision);
		}
	}
}

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Threading;

/// <summary>
/// 人物类
/// </summary>

namespace THUnity2D
{
	public enum JobType
	{
		job0 = 0,
		job1 = 1,
		job2 = 2
	}
	public sealed class Character : GameObject
	{
		public const int basicAp = 1000;
		public const int basicHp = 5000;
		public const int basicCD = 2000;
		public const int basicBulletNum = 15;

		public override GameObjType GetGameObjType()
		{
			return GameObjType.Character;
		}

		private long teamID = Team.invalidTeamID;
		public long TeamID
		{
			get => teamID;
			set
			{
				//Operations.Add
				lock (gameObjLock)
				{
					teamID = value;
					Debug(this, " joins in the tream: " + value.ToString());
				}
			}
		}

		public readonly JobType jobType;

		public readonly object propLock = new object();
		private bool isModifyingProp = false;
		public bool IsModifyingProp
		{
			get => isModifyingProp;
			set
			{
				lock (gameObjLock)
				{
					isModifyingProp = value;
				}
			}
		}

		private int cd;							//人物装弹固有CD
		public int CD { get => cd; }

		private int maxBulletNum;               //人物最大子弹数
		public int MaxBulletNum { get => maxBulletNum; }

		private int bulletNum;                  //目前持有的子弹数
		public int BulletNum { get => bulletNum; }
		private bool TrySubBulletNum()              //尝试将子弹数量减1
		{
			//Operations.Add
			lock (gameObjLock)
			{
				if (bulletNum > 0)
				{
					--bulletNum;
					return true;
				}
				return false;
			}
		}
		public void AddBulletNum()              //子弹数量加1
		{
			//Operations.Add
			lock (gameObjLock)
			{
				if (bulletNum < maxBulletNum) ++bulletNum;
			}
		}

		private readonly int maxHp;				//最大血量
		public int MaxHp { get => maxHp; }

		private int hp;							//当前血量
		public int HP { get => hp; }
		public void AddHp(int add)				//加血
		{
			//Operations.Add
			lock (gameObjLock)
			{
				hp = Math.Min(MaxHp, hp + add);
				Debug(this, " hp has added to: " + hp.ToString());
			}
		}
		private void SubHp(int sub)				//扣血
		{
			//Operations.Add
			lock (gameObjLock)
			{
				hp = Math.Max(0, hp - sub);
				Debug(this, " hp has subed to: " + hp.ToString());
			}
		}

		private int lifeNum = 0;				//自己的第几条命，记录死亡次数
		public int LifeNum { get => lifeNum; }
		private void AddLifeNum()
		{
			lock (gameObjLock)
			{
				++lifeNum;
			}
		}

		public void BeAttack(int subHP)
		{
			SubHp(subHP);
		}

		public readonly int orgAp;      //固有攻击力
		private int ap;                 //当前攻击力
		public int AP
		{
			get => ap;
			set
			{
				//Operations.Add
				lock (gameObjLock)
				{
					ap = value;
					Debug(this, "'s AP has been set to: " + value.ToString());
				}
			}
		}

		private Prop? holdProp;			//持有的道具
		public Prop? HoldProp
		{
			get => holdProp;
			set
			{
				//Operations.Add
				lock (gameObjLock)
				{
					holdProp = value;
					Debug(this, " picked the prop: " + (holdProp == null ? "null" : holdProp.ToString()));
				}
			}
		}

		public readonly BulletType bulletType;	//人物的发射子弹类型，射程伤害等信息存在子弹里

		public bool Attack()					//进行一次攻击
		{
			if (TrySubBulletNum()) return true;
			else return false;
		}

		public Prop? UseProp()					//使用手中道具，将道具返回给外部
		{
			lock (gameObjLock)
			{
				var oldProp = holdProp;
				holdProp = null;
				return oldProp;
			}
		}

		private int score;						//当前分数
		public int Score { get => score; }
		public void AddScore(int add)
		{
			//Operations.Add
				lock (gameObjLock)
				{
					score += add;
					Debug(this, " 's score has been added to: " + score.ToString());
				}
		}
		public void SubScore(int sub)
		{
			//Operations.Add
			lock (gameObjLock)
			{
				score -= sub;
				Debug(this, " 's score has been subed to: " + score.ToString());
			}
		}

		public override void Reset()
		{
			++lifeNum;
			base.Reset();
			hp = maxHp;
			ap = orgAp;
			holdProp = null;
			bulletNum = maxBulletNum;
		}

		//关于Buff的实现

		private enum BuffType : uint		//有哪些加成
		{
			MoveSpeed = 0u,
			AP = 1u,
			CD = 2u
		}
		private const uint BuffTypeNum = 3u;		//加成的种类个数，即enum BuffType的成员个数
		
		[StructLayout(LayoutKind.Explicit, Size = 8)]
		private struct BuffValue					//加成参数联合体类型，可能是int或double
		{
			[FieldOffset(0)]
			public int iValue;
			[FieldOffset(0)]
			public double lfValue;
			
			public BuffValue(int intValue) { this.lfValue = 0.0; this.iValue = intValue; }
			public BuffValue(double longFloatValue) { this.iValue = 0; this.lfValue = longFloatValue; }
		}

		private LinkedList<BuffValue>[] buffList;
		private object[] buffListLock;

		public void AddMoveSpeed(int add, int buffTime)
		{
			new Thread
				(
					() =>
					{
						BuffValue bf = new BuffValue(add);
						LinkedListNode<BuffValue> buffNode;
						lock (buffListLock[(uint)BuffType.MoveSpeed])
						{
							buffNode = buffList[(uint)BuffType.MoveSpeed].AddLast(bf);
						}
						ReCalculateMoveSpeed();
						Thread.Sleep(buffTime);
						try
						{
							lock (buffListLock[(uint)BuffType.MoveSpeed])
							{
								buffList[(uint)BuffType.MoveSpeed].Remove(buffNode);
							}
						}
						catch { }
						ReCalculateMoveSpeed();
					}
				)
			{ IsBackground = true }.Start();
		}

		private void ReCalculateMoveSpeed()
		{
			int res = orgMoveSpeed;
			lock (buffListLock[(uint)BuffType.MoveSpeed])
			{
				foreach (var add in buffList[(uint)BuffType.MoveSpeed])
				{
					res += add.iValue;
				}
			}
			MoveSpeed = Math.Max(Math.Min(res, MaxSpeed), MinSpeed);
		}

		public Character(XYPosition initPos, int radius, JobType jobType, int basicMoveSpeed) : base(initPos, radius, true, basicMoveSpeed, ShapeType.Circle)
		{

			buffList = new LinkedList<BuffValue>[BuffTypeNum];
			for (int i = 0; i < buffList.Length; ++i)
			{
				buffList[i] = new LinkedList<BuffValue>();
			}

			buffListLock = new object[5];
			for (int i = 0; i < buffListLock.Length; ++i)
			{
				buffListLock[i] = new object();
			}

			score = 0;
			this.jobType = jobType;

			switch (jobType)
			{
			case JobType.job0:
			default:
				cd = basicCD;
				maxBulletNum = basicBulletNum;
				bulletNum = maxBulletNum;
				maxHp = basicHp;
				hp = maxHp;
				orgAp = basicAp;
				ap = orgAp;
				holdProp = null;
				bulletType = BulletType.Empty;

				break;
			}

			Debug(this, " constructed!");
		}

	}
}

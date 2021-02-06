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
		public int CD
		{
			get => cd;
			private set
			{
				lock (gameObjLock)
				{
					cd = value;
					Debug(this, string.Format("'s AP has been set to: {0}.", value));
				}
			}
		}
		public readonly int orgCD;

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
				hp = Math.Max(Math.Min(MaxHp, hp + add), 0);
				Debug(this, " hp has added to: " + hp.ToString());
			}
		}
		private void SubHp(int sub)				//扣血
		{
			//Operations.Add
			lock (gameObjLock)
			{
				hp = Math.Min(Math.Max(0, hp - sub), MaxHp);
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
			if (!HasShield()) SubHp(subHP);
		}

		public const int MinAP = 0;
		public const int MaxAP = int.MaxValue;
		public readonly int orgAp;      //固有攻击力
		private int ap;                 //当前攻击力
		public int AP
		{
			get => ap;
			private set
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
			for (int i = 0; i < BuffTypeNum; ++i)
			{
				lock (buffListLock[i])
				{
					buffList[i].Clear();
				}
			}
		}

		#region 用于实现各种Buff道具的效果

		private enum BuffType : uint		//有哪些加成
		{
			MoveSpeed = 0u,
			AP = 1u,
			CD = 2u,
			Shield = 3u
		}
		private const uint BuffTypeNum = 4u;		//加成的种类个数，即enum BuffType的成员个数
		
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

		private void AddBuff(BuffValue bf, int buffTime, BuffType buffType, Action ReCalculateFunc)
		{
			new Thread
				(
					() =>
					{
						LinkedListNode<BuffValue> buffNode;
						lock (buffListLock[(uint)buffType])
						{
							buffNode = buffList[(uint)buffType].AddLast(bf);
						}
						ReCalculateFunc();
						Thread.Sleep(buffTime);
						try
						{
							lock (buffListLock[(uint)buffType])
							{
								buffList[(uint)buffType].Remove(buffNode);
							}
						}
						catch { }
						ReCalculateFunc();
					}
				)
			{ IsBackground = true }.Start();
		}

		public void AddMoveSpeed(int add, int buffTime)
		{
			AddBuff(new BuffValue(add), buffTime, BuffType.MoveSpeed, ReCalculateMoveSpeed);
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

		public void AddAP(int add, int buffTime)
		{
			AddBuff(new BuffValue(add), buffTime, BuffType.AP, ReCalculateAP);
		}
		private void ReCalculateAP()
		{
			int res = orgAp;
			lock (buffListLock[(uint)BuffType.AP])
			{
				foreach (var bf in buffList[(uint)BuffType.AP])
				{
					res += bf.iValue;
				}
			}
			AP = Math.Max(Math.Min(res, MaxAP), MinAP);
		}

		public void ChangeCD(double discount, int buffTime)
		{
			AddBuff(new BuffValue(discount), buffTime, BuffType.CD, ReCalculateCD);
		}
		private void ReCalculateCD()
		{
			double times = 1.0;
			lock (buffListLock[(uint)BuffType.CD])
			{
				foreach (var bf in buffList[(uint)BuffType.CD])
				{
					times *= bf.lfValue;
				}
			}
			CD = (int)(orgCD * times);
		}

		public void AddShield(int shieldTime)
		{
			AddBuff(new BuffValue(), shieldTime, BuffType.Shield, () => { });
		}
		public bool HasShield()
		{
			lock (buffListLock[(uint)BuffType.Shield])
			{
				return buffList[(uint)BuffType.Shield].Count != 0;
			}
		}

		#endregion

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
				orgCD = basicCD;
				cd = orgCD;
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

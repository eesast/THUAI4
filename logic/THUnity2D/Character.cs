using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Threading;

/// <summary>
/// 人物类
/// </summary>

namespace THUnity2D
{
	public enum JobType : int
	{
		Job0 = 0,
		Job1 = 1,
		Job2 = 2,
		Job3 = 3,
		Job4 = 4,
		Job5 = 5,
		Job6 = 6,
		InvalidJobType = int.MaxValue
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

		public void BeAttack(int subHP, bool hasSpear, Character? attacker)	//遭到攻击
		{
			if (hasSpear || !HasShield) SubHp(subHP);
			if (hp <= 0) TryActivatingTotem();
			if (jobType == JobType.Job6) attacker?.BeBounced(subHP / 2, this.HasSpear, this);   //职业6可以反弹伤害
		}

		private void BeBounced(int subHP, bool hasSpear, Character? attacker)	//遭到反弹
		{
			if (hasSpear || !HasShield) SubHp(subHP);
			if (hp <= 0) hp = 1;		//反弹不会致死
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

		private string message = "THUAI4";
		public string Message
		{
			get => message;
			set
			{
				lock (gameObjLock)
				{
					message = value;
				}
			}
		}

		#region 用于实现各种Buff道具的效果

		private enum BuffType : uint		//有哪些加成
		{
			MoveSpeed = 0u,
			AP = 1u,
			CD = 2u,
			Shield = 3u,
			Totem = 4u,
			Spear = 5u
		}
		private const uint BuffTypeNum = 6u;		//加成的种类个数，即enum BuffType的成员个数
		
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

		public void AddMoveSpeed(double add, int buffTime)
		{
			AddBuff(new BuffValue(add), buffTime, BuffType.MoveSpeed, ReCalculateMoveSpeed);
		}
		private int ReCalculateFloatBuff(BuffType buffType, int orgVal, int maxVal, int minVal)
		{
			double times = 1.0;
			lock (buffListLock[(uint)buffType])
			{
				foreach (var add in buffList[(uint)buffType])
				{
					times *= add.lfValue;
				}
			}
			return Math.Max(Math.Min((int)Math.Round(orgVal * times), maxVal), minVal);
		}
		private void ReCalculateMoveSpeed()
		{
			MoveSpeed = ReCalculateFloatBuff(BuffType.MoveSpeed, OrgMoveSpeed, MaxSpeed, MinSpeed);
		}

		public void AddAP(double add, int buffTime)
		{
			AddBuff(new BuffValue(add), buffTime, BuffType.AP, ReCalculateAP);
		}
		private void ReCalculateAP()
		{
			AP = ReCalculateFloatBuff(BuffType.AP, orgAp, MaxAP, MinAP);
		}

		public void ChangeCD(double discount, int buffTime)
		{
			AddBuff(new BuffValue(discount), buffTime, BuffType.CD, ReCalculateCD);
		}
		private void ReCalculateCD()
		{
			CD = ReCalculateFloatBuff(BuffType.CD, orgCD, int.MaxValue, 1);
		}

		public void AddShield(int shieldTime)
		{
			AddBuff(new BuffValue(), shieldTime, BuffType.Shield, () => { });
		}
		public bool HasShield
		{
			get
			{
				lock (buffListLock[(uint)BuffType.Shield])
				{
					return buffList[(uint)BuffType.Shield].Count != 0;
				}
			}
		}

		public void AddTotem(int totemTime)
		{
			AddBuff(new BuffValue(), totemTime, BuffType.Totem, () => { });
		}
		public bool HasTotem
		{
			get
			{
				lock (buffListLock[(uint)BuffType.Totem])
				{
					return buffList[(uint)BuffType.Totem].Count != 0;
				}
			}
		}
		private void TryActivatingTotem()
		{
			if (HasTotem)
			{
				hp = maxHp;
				lock (buffListLock[(uint)BuffType.Totem])
				{
					buffList[(uint)BuffType.Totem].Clear();
				}
			}
		}

		public void AddSpear(int spearTime)
		{
			AddBuff(new BuffValue(), spearTime, BuffType.Spear, () => { });
		}
		public bool HasSpear
		{
			get
			{
				lock (buffListLock[(uint)BuffType.Spear])
				{
					return buffList[(uint)BuffType.Spear].Count != 0;
				}
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

			buffListLock = new object[buffList.Length];
			for (int i = 0; i < buffListLock.Length; ++i)
			{
				buffListLock[i] = new object();
			}

			score = 0;
			this.jobType = jobType;

			switch (jobType)
			{
			default:
			case JobType.Job0:
				cd = orgCD = basicCD;
				bulletNum = maxBulletNum = basicBulletNum;
				hp = maxHp = basicHp;
				ap = orgAp = basicAp;
				holdProp = null;
				bulletType = BulletType.Bullet0;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
				break;
			case JobType.Job1:
				cd = orgCD = basicCD;
				bulletNum = maxBulletNum = basicBulletNum;
				hp = maxHp = basicHp;
				ap = orgAp = basicAp;
				holdProp = null;
				bulletType = BulletType.Bullet1;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
				break;
			case JobType.Job2:
				cd = orgCD = basicCD * 2;
				bulletNum = maxBulletNum = basicBulletNum;
				hp = maxHp = basicHp;
				ap = orgAp = basicAp * 2;
				holdProp = null;
				bulletType = BulletType.Bullet2;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2 / 3;
				break;
			case JobType.Job3:
				cd = orgCD = basicCD * 2;
				bulletNum = maxBulletNum = basicBulletNum / 2;
				hp = maxHp = basicHp / 2;
				ap = orgAp = basicAp / 2;
				holdProp = null;
				bulletType = BulletType.Bullet3;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 3 / 2;
				break;
			case JobType.Job4:
				cd = orgCD = basicCD * 2;
				bulletNum = maxBulletNum = basicBulletNum / 3;
				hp = maxHp = basicHp * 2 / 3;
				ap = orgAp = basicAp * 3;
				holdProp = null;
				bulletType = BulletType.Bullet4;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2;
				break;
			case JobType.Job5:
				cd = orgCD = basicCD * 2;
				bulletNum = maxBulletNum = basicBulletNum / 3;
				hp = maxHp = basicHp * 2 / 3;
				ap = orgAp = basicAp * 3;
				holdProp = null;
				bulletType = BulletType.Bullet5;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2;
				break;
			case JobType.Job6:
				cd = orgCD = basicCD;
				bulletNum = maxBulletNum = basicBulletNum;
				hp = maxHp = basicHp * 3;
				ap = orgAp = basicAp;
				holdProp = null;
				bulletType = BulletType.Bullet6;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 3 / 4;
				break;
			}

			Debug(this, " constructed!");
		}

	}
}

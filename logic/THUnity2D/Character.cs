using System;

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
	public sealed partial class Character : GameObject
	{
		public const int basicAp = 1000;
		public const int basicHp = 6000;
		public const int basicCD = 1000;
		public const int basicBulletNum = 12;

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
		public int MaxBulletNum => maxBulletNum;

		private int bulletNum;                  //目前持有的子弹数
		public int BulletNum => bulletNum;
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
		public int MaxHp => maxHp;

		private int hp;							//当前血量
		public int HP => hp;
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
		public int LifeNum => lifeNum;
		private void AddLifeNum()
		{
			lock (gameObjLock)
			{
				++lifeNum;
			}
		}

		public void BeAttack(int subHP, bool hasSpear, Character? attacker)	//遭到攻击
		{
			if (attacker.TeamID != this.TeamID)
			{
				if (hasSpear || !HasShield) SubHp(subHP);
				if (hp <= 0) TryActivatingTotem();
				if (jobType == JobType.Job6) attacker?.BeBounced(subHP * 3 / 4, this.HasSpear, this);   //职业6可以反弹伤害
			}
			else if (attacker?.jobType == JobType.Job6)
			{
				AddHp(subHP * 6);				// 职业六回血6倍
			}
		}

		private void BeBounced(int subHP, bool hasSpear, Character? attacker)	//遭到反弹
		{
			if (hasSpear || !HasShield)
			{
				var hpBeforeBounce = hp;
				if (hpBeforeBounce - subHP <= 0)
				{
					subHP = hpBeforeBounce - 1;
				}
				SubHp(subHP);
			}
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
		public int Score => score;
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
			bulletNum = maxBulletNum / 2;
			buffManeger.ClearAll();
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

		#region 各种Buff道具效果的接口

		public void AddMoveSpeed(double add, int buffTime) => buffManeger.AddMoveSpeed(add, buffTime, newVal => { MoveSpeed = newVal; }, OrgMoveSpeed);

		public void AddAP(double add, int buffTime) => buffManeger.AddAP(add, buffTime, newVal => { AP = newVal; }, orgAp);

		public void ChangeCD(double discount, int buffTime) => buffManeger.ChangeCD(discount, buffTime, newVal => { CD = newVal; }, orgCD);

		public void AddShield(int shieldTime) => buffManeger.AddShield(shieldTime);
		public bool HasShield => buffManeger.HasShield;

		public void AddTotem(int totemTime) => buffManeger.AddTotem(totemTime);
		public bool HasTotem => buffManeger.HasTotem;

		public void AddSpear(int spearTime) => buffManeger.AddSpear(spearTime);
		public bool HasSpear => buffManeger.HasSpear;

		private void TryActivatingTotem()
		{
			if (buffManeger.TryActivatingTotem())
			{
				hp = maxHp;
			}
		}

		#endregion

		public Character(XYPosition initPos, int radius, JobType jobType, int basicMoveSpeed) : base(initPos, radius, true, basicMoveSpeed, ShapeType.Circle)
		{
			buffManeger = new BuffManeger();

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
				hp = maxHp = basicHp * 4 / 3;
				ap = orgAp = basicAp * 3 / 4;
				holdProp = null;
				bulletType = BulletType.Bullet1;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
				break;
			case JobType.Job2:
				cd = orgCD = basicCD * 2;
				bulletNum = maxBulletNum = basicBulletNum * 2 / 3;
				hp = maxHp = basicHp;
				ap = orgAp = basicAp * 5 / 4;
				holdProp = null;
				bulletType = BulletType.Bullet2;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed / 3;
				break;
			case JobType.Job3:
				cd = orgCD = basicCD * 3;
				bulletNum = maxBulletNum = basicBulletNum / 2;
				hp = maxHp = basicHp * 2 / 5;
				ap = orgAp = basicAp * 3 / 8;
				holdProp = null;
				bulletType = BulletType.Bullet3;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 3 / 2;
				break;
			case JobType.Job4:
				cd = orgCD = basicCD * 4;
				bulletNum = maxBulletNum = basicBulletNum / 4;
				hp = maxHp = basicHp * 2 / 3;
				ap = orgAp = basicAp * 7 / 2;
				holdProp = null;
				bulletType = BulletType.Bullet4;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2;
				break;
			case JobType.Job5:
				cd = orgCD = basicCD * 2;
				bulletNum = maxBulletNum = basicBulletNum / 3;
				hp = maxHp = basicHp * 2 / 3;
				ap = orgAp = basicAp * 4;
				holdProp = null;
				bulletType = BulletType.Bullet5;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2;
				break;
			case JobType.Job6:
				cd = orgCD = basicCD;
				bulletNum = maxBulletNum = basicBulletNum;
				hp = maxHp = basicHp * 2;
				ap = orgAp = basicAp / 2;
				holdProp = null;
				bulletType = BulletType.Bullet6;
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
				break;
			}

			Debug(this, " constructed!");
		}

	}
}

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
	public abstract partial class Character : GameObject, ICharacter
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

		protected int moveSpeed;
		public int MoveSpeed
		{
			get => moveSpeed;
			protected set
			{
				lock (gameObjLock)
				{
					moveSpeed = value;
				}
			}
		}

		private int orgMoveSpeed;
		public int OrgMoveSpeed { get => orgMoveSpeed; protected set { orgMoveSpeed = value; } }

		public abstract JobType Job { get; }

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

		public override bool IsRigid => true;

		protected int cd;							//人物装弹固有CD
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
		public int OrgCD { get; protected set; }

		protected int maxBulletNum;               //人物最大子弹数
		public int MaxBulletNum => maxBulletNum;

		protected int bulletNum;                  //目前持有的子弹数
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

		public int MaxHp { get; protected set; }				//最大血量

		protected int hp;						//当前血量
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

		private object beAttackedLock = new object();
		public bool BeAttack(int subHP, bool hasSpear, Character? attacker)	//遭到攻击，如果因为此次攻击致死则返回 true
		{
			lock (beAttackedLock)
			{
				if (hp <= 0) return false;
				if (!(attacker?.TeamID == this.TeamID))
				{
					if (hasSpear || !HasShield) SubHp(subHP);
					if (hp <= 0) TryActivatingTotem();
					if (Job == JobType.Job6) attacker?.BeBounced(subHP * 3 / 4, this.HasSpear, this);   //职业6可以反弹伤害
				}
				else if (attacker?.Job == JobType.Job6)
				{
					AddHp(subHP * 6);               // 职业六回血6倍
				}
				return hp <= 0;
			}	
		}

		private void BeBounced(int subHP, bool hasSpear, Character? bouncer)	//遭到反弹
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
		public int OrgAp { get; protected set; }		//固有攻击力
		protected int ap;								//当前攻击力
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

		public abstract BulletType Bullet { get; } //人物的发射子弹类型，射程伤害等信息存在子弹里

		public Bullet? Attack(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)		//进行一次攻击
		{
			if (TrySubBulletNum()) return ProduceOneBullet(posOffset, bulletRadius, basicBulletMoveSpeed);
			else return null;
		}
		protected abstract Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed);

		public Prop? UseProp()					//使用手中道具，将道具返回给外部
		{
			lock (gameObjLock)
			{
				var oldProp = holdProp;
				holdProp = null;
				return oldProp;
			}
		}

		bool IMovable.IgnoreCollide(IGameObj targetObj)
		{
			if (targetObj is BirthPoint && object.ReferenceEquals(((BirthPoint)targetObj).Parent, this))         // 自己的出生点可以忽略碰撞
			{
				return true;
			}
			else if (targetObj is Mine && ((Mine)targetObj).Parent?.TeamID == TeamID)          // 自己队的炸弹忽略碰撞
			{
				return true;
			}
			return false;
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
			AddLifeNum();
			base.Reset();
			this.moveSpeed = orgMoveSpeed;
			hp = MaxHp;
			ap = OrgAp;
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

		public void AddAP(double add, int buffTime) => buffManeger.AddAP(add, buffTime, newVal => { AP = newVal; }, OrgAp);

		public void ChangeCD(double discount, int buffTime) => buffManeger.ChangeCD(discount, buffTime, newVal => { CD = newVal; }, OrgCD);

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
				hp = MaxHp;
			}
		}

		#endregion

		/// <summary>
		/// Construct a character
		/// </summary>
		/// <returns>A handle to the character</returns>
		static public Character? GetCharacter(XYPosition initPos, int radius, int basicMoveSpeed, JobType job)
		{
			switch (job)
			{
				case JobType.Job0: return new Character0(initPos, radius, basicMoveSpeed);
				case JobType.Job1: return new Character1(initPos, radius, basicMoveSpeed);
				case JobType.Job2: return new Character2(initPos, radius, basicMoveSpeed);
				case JobType.Job3: return new Character3(initPos, radius, basicMoveSpeed);
				case JobType.Job4: return new Character4(initPos, radius, basicMoveSpeed);
				case JobType.Job5: return new Character5(initPos, radius, basicMoveSpeed);
				case JobType.Job6: return new Character6(initPos, radius, basicMoveSpeed);
			}
			return null;
		}

		public Character(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, ShapeType.Circle)
		{
			buffManeger = new BuffManeger();

			score = 0;
			holdProp = null;

			Debug(this, " constructed!");
		}

	}

	internal sealed class Character0 : Character
	{
		public Character0(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, basicMoveSpeed)
		{
			cd = OrgCD = basicCD;
			bulletNum = maxBulletNum = basicBulletNum;
			hp = MaxHp = basicHp;
			ap = OrgAp = basicAp;
			MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
		}

		public override JobType Job => JobType.Job0;
		public override BulletType Bullet => BulletType.Bullet0;

		protected override Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)
		{
			return new Bullet0(Position + posOffset, bulletRadius, basicBulletMoveSpeed, ap, HasSpear);
		}
	}

	internal sealed class Character1 : Character
	{
		public Character1(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, basicMoveSpeed)
		{
			cd = OrgCD = basicCD;
			bulletNum = maxBulletNum = basicBulletNum;
			hp = MaxHp = basicHp * 4 / 3;
			ap = OrgAp = basicAp * 3 / 4;
			MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
		}

		public override JobType Job => JobType.Job1;
		public override BulletType Bullet => BulletType.Bullet1;

		protected override Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)
		{
			return new Bullet1(Position + posOffset, bulletRadius, basicBulletMoveSpeed, ap, HasSpear);
		}
	}

	internal sealed class Character2 : Character
	{
		public Character2(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, basicMoveSpeed)
		{
			cd = OrgCD = basicCD * 2;
			bulletNum = maxBulletNum = basicBulletNum * 2 / 3;
			hp = MaxHp = basicHp;
			ap = OrgAp = basicAp * 5 / 4;
			MoveSpeed = OrgMoveSpeed = basicMoveSpeed / 3;
		}

		public override JobType Job => JobType.Job2;
		public override BulletType Bullet => BulletType.Bullet2;

		protected override Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)
		{
			return new Bullet2(Position + posOffset, bulletRadius, basicBulletMoveSpeed, ap, HasSpear);
		}
	}

	internal sealed class Character3 : Character
	{
		public Character3(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, basicMoveSpeed)
		{
			cd = OrgCD = basicCD * 3;
			bulletNum = maxBulletNum = basicBulletNum / 2;
			hp = MaxHp = basicHp * 2 / 5;
			ap = OrgAp = basicAp * 3 / 8;
			MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 3 / 2;
		}

		public override JobType Job => JobType.Job3;
		public override BulletType Bullet => BulletType.Bullet3;

		protected override Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)
		{
			return new Bullet3(Position + posOffset, bulletRadius, basicBulletMoveSpeed, ap, HasSpear);
		}
	}

	internal sealed class Character4 : Character
	{
		public Character4(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, basicMoveSpeed)
		{
			cd = OrgCD = basicCD * 4;
			bulletNum = maxBulletNum = basicBulletNum / 4;
			hp = MaxHp = basicHp * 2 / 3;
			ap = OrgAp = basicAp * 7 / 2;
			MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2;
		}

		public override JobType Job => JobType.Job4;
		public override BulletType Bullet => BulletType.Bullet4;

		protected override Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)
		{
			return new Bullet4(Position + posOffset, bulletRadius, basicBulletMoveSpeed, ap, HasSpear);
		}
	}

	internal sealed class Character5 : Character
	{
		public Character5(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, basicMoveSpeed)
		{
			cd = OrgCD = basicCD * 2;
			bulletNum = maxBulletNum = basicBulletNum / 3;
			hp = MaxHp = basicHp * 2 / 3;
			ap = OrgAp = basicAp * 4;
			MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2;
		}

		public override JobType Job => JobType.Job5;
		public override BulletType Bullet => BulletType.Bullet5;

		protected override Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)
		{
			return new Bullet5(Position + posOffset, bulletRadius, basicBulletMoveSpeed, ap, HasSpear);
		}
	}

	internal sealed class Character6 : Character
	{
		public Character6(XYPosition initPos, int radius, int basicMoveSpeed) : base(initPos, radius, basicMoveSpeed)
		{
			cd = OrgCD = basicCD;
			bulletNum = maxBulletNum = basicBulletNum;
			hp = MaxHp = basicHp * 2;
			ap = OrgAp = basicAp / 2;
			MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
		}

		public override JobType Job => JobType.Job6;
		public override BulletType Bullet => BulletType.Bullet6;

		protected override Bullet ProduceOneBullet(XYPosition posOffset, int bulletRadius, int basicBulletMoveSpeed)
		{
			return new Bullet6(Position + posOffset, bulletRadius, basicBulletMoveSpeed, ap, HasSpear);
		}
	}
}

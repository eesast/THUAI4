using System;
using System.Collections.Generic;
using System.Text;

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
		private readonly int _orgMoveSpeed;         //人物固有移动速度
		public void ResetMoveSpeed()                //重设人物速度为初始速度
		{
			MoveSpeed = _orgMoveSpeed;
		}

		public readonly JobType jobType;

		private int cd;							//人物装弹固有CD
		public int CD { get => cd; }

		private int maxBulletNum;               //人物最大子弹数
		public int MaxBulletNum { get => maxBulletNum; }

		private int bulletNum;                  //目前持有的子弹数
		public int BulletNum { get => bulletNum; }
		private bool TrySubBulletNum()              //尝试将子弹数量减1
		{
			if (bulletNum > 0)
			{
				Operations.Add
				(
					() =>
					{
						if (bulletNum > 0) --bulletNum;
					}
				);
				return true;
			}
			else return false;
		}
		public void AddBulletNum()              //子弹数量加1
		{
			Operations.Add
				(
					() =>
					{
						if (bulletNum < maxBulletNum) ++bulletNum;
					}
				);
		}

		private readonly int maxHp;				//最大血量
		public int MaxHp { get => maxHp; }

		private int hp;							//当前血量
		public int HP { get => hp; }
		public void AddHp(int add)				//加血
		{
			Operations.Add
				(
					() =>
					{
						hp = Math.Min(MaxHp, hp + add);
						Debug(this, " hp has added to: " + hp.ToString());
					}
				);
		}
		public void SubHp(int sub)				//扣血
		{
			Operations.Add
				(
					() =>
					{
						hp = Math.Max(0, hp - sub);
						Debug(this, " hp has subed to: " + hp.ToString());
					}
				);
		}

		public readonly int orgAp;      //固有攻击力
		private int ap;                 //当前攻击力
		public int AP
		{
			get => ap;
			set
			{
				Operations.Add
					(
						() =>
						{
							ap = value;
							Debug(this, "'s AP has been set to: " + value.ToString());
						}
					);
			}
		}

		private Prop? holdProp;			//持有的道具
		public Prop? HoldProp
		{
			get => holdProp;
			set
			{
				Operations.Add
					(
						() =>
						{
							holdProp = value;
							Debug(this, " picked the prop: " + holdProp.ToString());
						}
					);
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
			var oldProp = HoldProp;
			HoldProp = null;
			return oldProp;
		}

		private int score;						//当前分数
		public int Score { get => score; }
		public void AddScore(int add)
		{
			Operations.Add
				(
					() =>
					{
						score += add;
						Debug(this, " 's score has been added to: " + score.ToString());
					}
				);
		}
		public void SubScore(int sub)
		{
			Operations.Add
				(
					() =>
					{
						score -= sub;
						Debug(this, " 's score has been subed to: " + score.ToString());
					}
				);
		}

		public Character(XYPosition initPos, int radius, JobType jobType, int basicMoveSpeed) : base(initPos, radius, true, basicMoveSpeed)
		{
			score = 0;
			this.jobType = jobType;

			switch (jobType)
			{
			case JobType.job0:
				_orgMoveSpeed = basicMoveSpeed;
				cd = 1500;
				maxBulletNum = 10;
				bulletNum = maxBulletNum;
				maxHp = 5000;
				hp = maxHp;
				orgAp = 1000;
				ap = orgAp;
				holdProp = null;
				bulletType = BulletType.empty;

				break;
			}
		}

	}
}

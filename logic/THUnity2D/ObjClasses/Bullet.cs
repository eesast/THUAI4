namespace THUnity2D
{
	public enum BulletType
	{
		Bullet0 = 0,
		Bullet1 = 1,
		Bullet2 = 2,
		Bullet3 = 3,
		Bullet4 = 4,
		Bullet5 = 5,
		Bullet6 = 6
	}
	public abstract class Bullet : Obj, IMovable
	{

		protected int moveSpeed;
		/// <summary>
		/// 移动速度
		/// </summary>
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


		private int ap;
		/// <summary>
		/// //攻击力
		/// </summary>
		public int AP => ap;

		private readonly bool hasSpear;
		/// <summary>
		/// 是否有矛
		/// </summary>
		public bool HasSpear => hasSpear;

		/// <summary>
		/// 是否能沿途染色
		/// </summary>
		public virtual bool CanColorPath { get => false; }

		/// <summary>
		/// 子弹种类
		/// </summary>
		public abstract BulletType BulletX { get; }

		/// <summary>
		/// 获取一个正方形区域，中心为0，其他位置的数字为相对于中心的偏移量
		/// </summary>
		/// <param name="edgeLen">正方形边长，要求为奇数</param>
		/// <returns>正方形区域的数组</returns>
		private XYPosition[] GetSquareRange(uint edgeLen)
		{
			XYPosition[] range = new XYPosition[edgeLen * edgeLen];
			int offset = (int)(edgeLen >> 1);
			for (int i = 0; i < (int)edgeLen; ++i)
			{
				for (int j = 0; j < (int)edgeLen; ++j)
				{
					range[i * edgeLen + j].x = i - offset;
					range[i * edgeLen + j].y = j - offset;
				}
			}
			return range;
		}

		protected abstract uint ColorRangeEdgeLength { get; }	// 染色区域边长
		protected abstract uint AttackRangeEdgeLength { get; }	// 攻击区域边长
		public XYPosition[] GetColorRange()     //返回染色范围，相对自己的相对距离
		{
			return GetSquareRange(ColorRangeEdgeLength);
		}
		public XYPosition[] GetAttackRange()    //返回爆炸范围，相对自己的相对距离
		{
			return GetSquareRange(AttackRangeEdgeLength);
		}

		bool IMovable.IgnoreCollide(IGameObj targetObj)
		{
			if (targetObj is BirthPoint || targetObj is Mine) return true;		// 子弹不会与出生点和道具碰撞
			return false;
		}

		public virtual void BeforeShooting(ref int timeInMilliseconds, ref double angle) { }

		public Bullet(XYPosition initPos, int radius, int ap, bool hasSpear)
			: base(initPos, radius, ObjType.Bullet, ShapeType.Circle)
		{
			this.ap = ap;
			this.hasSpear = hasSpear;
		}

	}

	internal sealed class Bullet0 : Bullet
	{
		public Bullet0(XYPosition initPos, int radius, int basicMoveSpeed, int ap, bool hasSpear) : base(initPos, radius, ap, hasSpear)
		{
			MoveSpeed = basicMoveSpeed;
		}
		public override bool IsRigid => true;
		public override BulletType BulletX => BulletType.Bullet0;
		protected override uint ColorRangeEdgeLength => 3;
		protected override uint AttackRangeEdgeLength => 3;

		public override void BeforeShooting(ref int timeInMilliseconds, ref double angle)
		{
			timeInMilliseconds = int.MaxValue;
		}
	}

	internal sealed class Bullet1 : Bullet
	{
		public Bullet1(XYPosition initPos, int radius, int basicMoveSpeed, int ap, bool hasSpear) : base(initPos, radius, ap, hasSpear)
		{
			MoveSpeed = basicMoveSpeed * 2;
		}
		public override bool IsRigid => false;
		public override BulletType BulletX => BulletType.Bullet1;
		protected override uint ColorRangeEdgeLength => 3;
		protected override uint AttackRangeEdgeLength => 3;
	}

	internal sealed class Bullet2 : Bullet
	{
		public Bullet2(XYPosition initPos, int radius, int basicMoveSpeed, int ap, bool hasSpear) : base(initPos, radius, ap, hasSpear)
		{
			MoveSpeed = basicMoveSpeed / 2;
		}
		public override bool IsRigid => true;
		public override BulletType BulletX => BulletType.Bullet2;
		protected override uint ColorRangeEdgeLength => 5;
		protected override uint AttackRangeEdgeLength => 7;
	}

	internal sealed class Bullet3 : Bullet
	{
		public Bullet3(XYPosition initPos, int radius, int basicMoveSpeed, int ap, bool hasSpear) : base(initPos, radius, ap, hasSpear)
		{
			MoveSpeed = basicMoveSpeed / 2;
		}
		public override bool IsRigid => true;
		public override BulletType BulletX => BulletType.Bullet3;
		protected override uint ColorRangeEdgeLength => 5;
		protected override uint AttackRangeEdgeLength => 1;

		public override bool CanColorPath => true;

		public override void BeforeShooting(ref int timeInMilliseconds, ref double angle)
		{
			timeInMilliseconds = int.MaxValue;
		}
	}

	internal sealed class Bullet4 : Bullet
	{
		public Bullet4(XYPosition initPos, int radius, int basicMoveSpeed, int ap, bool hasSpear) : base(initPos, radius, ap, hasSpear)
		{
			MoveSpeed = basicMoveSpeed * 3;
		}
		public override bool IsRigid => false;
		public override BulletType BulletX => BulletType.Bullet4;
		protected override uint ColorRangeEdgeLength => 1;
		protected override uint AttackRangeEdgeLength => 7;
	}

	internal sealed class Bullet5 : Bullet
	{
		public Bullet5(XYPosition initPos, int radius, int basicMoveSpeed, int ap, bool hasSpear) : base(initPos, radius, ap, hasSpear)
		{
			MoveSpeed = basicMoveSpeed;
		}
		public override bool IsRigid => true;
		public override BulletType BulletX => BulletType.Bullet5;
		protected override uint ColorRangeEdgeLength => 3;
		protected override uint AttackRangeEdgeLength => 3;

		public override void BeforeShooting(ref int timeInMilliseconds, ref double angle)
		{
			timeInMilliseconds = 0;
		}

	}

	internal sealed class Bullet6 : Bullet
	{
		public Bullet6(XYPosition initPos, int radius, int basicMoveSpeed, int ap, bool hasSpear) : base(initPos, radius, ap, hasSpear)
		{
			MoveSpeed = basicMoveSpeed;
		}
		public override bool IsRigid => true;
		public override BulletType BulletX => BulletType.Bullet6;
		protected override uint ColorRangeEdgeLength => 1;
		protected override uint AttackRangeEdgeLength => 3;

		public override void BeforeShooting(ref int timeInMilliseconds, ref double angle)
		{
			timeInMilliseconds = int.MaxValue;
		}
	}
}

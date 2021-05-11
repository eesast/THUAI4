namespace THUnity2D
{

	public enum PropType
	{
		Null = 0,
		Bike = 1,
		Amplifier = 2,
		JinKeLa = 3,
		Rice = 4,
		Shield = 5,
		NegativeFeedback = Shield,		// !!!NegativeFeedBack 必须在下面，否则转换成通信类型时失败
		Totem = 6,
		Spear = 7,
		Phaser = Spear,					// !!!Phaser 必须在下面，否则转换成通信类型时失败
		Dirt = 8,
		Attenuator = 9,
		Divider = 10
	}

	public abstract class Prop : Obj, IMovable
	{
		public const int MinPropTypeNum = 1;
		public const int MaxPropTypeNum = 10;

		protected bool laid = false;		//道具是否已放置
		public bool Laid => laid;

		public override bool IsRigid => true;

		public abstract PropType GetPropType();

		protected int moveSpeed = 0;
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

		public void ResetPosition(XYPosition pos)
		{
			Position = pos;
		}
		public void ResetMoveSpeed(int newMoveSpeed)
		{
			MoveSpeed = newMoveSpeed;
		}

		bool IMovable.IgnoreCollide(IGameObj targetObj) => true;

		public Prop(XYPosition initPos, int radius) : base(initPos, radius, ObjType.Prop, ShapeType.Square) { }
	}

	public abstract class Buff : Prop
	{
		public Buff(XYPosition initPos, int radius) : base(initPos, radius) { }
	}

	public abstract class Mine : Prop
	{
		public void SetLaid(XYPosition pos)
		{
			if (laid) return;
			laid = true;
			Position = pos;
			shape = ShapeType.Circle;
		}
		public Mine(XYPosition initPos, int radius) : base(initPos, radius) { }

	}

	public sealed class Bike : Buff
	{

		public Bike(XYPosition initPos, int radius) : base(initPos, radius) { }
		
		public override PropType GetPropType()
		{
			return PropType.Bike;
		}
	}
	
	public sealed class Amplifier : Buff
	{

		public Amplifier(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Amplifier;
		}
	}

	public sealed class JinKeLa : Buff
	{

		public JinKeLa(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.JinKeLa;
		}
	}

	public sealed class Rice : Buff
	{

		public Rice(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Rice;
		}
	}

	public sealed class Shield : Buff
	{
		public Shield(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Shield;
		}
	}

	public sealed class Totem : Buff
	{
		public Totem(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Totem;
		}
	}

	public sealed class Spear : Buff
	{
		public Spear(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Spear;
		}
	}

	public sealed class Dirt : Mine
	{
		public Dirt(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Dirt;
		}
	}

	public sealed class Attenuator : Mine
	{
		public Attenuator(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Attenuator;
		}
	}

	public sealed class Divider : Mine
	{
		public Divider(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Divider;
		}
	}
}

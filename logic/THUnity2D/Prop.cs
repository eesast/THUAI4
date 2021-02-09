using System;
using System.Collections.Generic;
using System.Text;
using THUnity2D;

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
		NegativeFeedback = Shield,
		Totem = 6,
		Spear = 7,
		Phaser = Spear,
		Dirt = 8,
		Attenuator = 9,
		Divider = 10
	}

	public abstract class Prop : Obj
	{
		public const int MinPropTypeNum = 1;
		public const int MaxPropTypeNum = 10;

		protected bool laid = false;		//道具是否已放置
		public bool Laid { get => laid; }

		public abstract PropType GetPropType();
		//道具类
		public Prop(XYPosition initPos, int radius) : base(initPos, radius, true, 0, ObjType.Prop, ShapeType.Sqare) { }
	}

	public abstract class Buff : Prop
	{
		public Buff(XYPosition initPos, int radius) : base(initPos, radius) { }
	}

	public abstract class Mine : Prop
	{
		public Mine(XYPosition initPos, int radius) : base(initPos, radius) { }

		public void SetLaid(XYPosition pos)
		{
			if (laid) return;
			laid = true;
			position = pos;
			shape = ShapeType.Circle;
		}
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

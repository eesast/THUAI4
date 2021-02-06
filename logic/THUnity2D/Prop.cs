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
		Totem = 6
	}

	public abstract class Prop : Obj
	{
		public const int MinPropTypeNum = 1;
		public const int MaxPropTypeNum = 6;

		private bool laid = false;		//道具是否已放置
		public bool Laid { get => laid; }
		public void SetLaid() { laid = true; }

		public abstract PropType GetPropType();
		//道具类
		public Prop(XYPosition initPos, int radius) : base(initPos, radius, true, 0, ObjType.Prop, ShapeType.Sqare) { }
	}

	public abstract class Buff : Prop
	{
		public Buff(XYPosition initPos, int radius) : base(initPos, radius) { }
	}

	public class Bike : Buff
	{
		private readonly int moveSpeedBuff;
		public int MoveSpeedBuff { get => moveSpeedBuff; }

		public Bike(XYPosition initPos, int radius, int moveSpeedBuff) : base(initPos, radius)
		{
			this.moveSpeedBuff = moveSpeedBuff;
		}
		
		public override PropType GetPropType()
		{
			return PropType.Bike;
		}
	}
	
	public class Amplifier : Buff
	{
		private readonly int atkBuff;
		public int AtkBuff { get => atkBuff; }
		public Amplifier(XYPosition initPos, int radius, int atkBuff) : base(initPos, radius)
		{
			this.atkBuff = atkBuff;
		}

		public override PropType GetPropType()
		{
			return PropType.Amplifier;
		}
	}

	public class JinKeLa : Buff
	{
		private readonly double cdDiscount;
		public double CdDiscoount { get => cdDiscount; }

		public JinKeLa(XYPosition initPos, int radius, double cdDiscount) : base(initPos, radius)
		{
			this.cdDiscount = cdDiscount;
		}

		public override PropType GetPropType()
		{
			return PropType.JinKeLa;
		}
	}

	public class Rice : Buff
	{
		private readonly int hpAdd;
		public int HPAdd { get => hpAdd; }

		public Rice(XYPosition initPos, int radius, int hpAdd) : base(initPos, radius)
		{
			this.hpAdd = hpAdd;
		}

		public override PropType GetPropType()
		{
			return PropType.Rice;
		}
	}

	public class Shield : Buff
	{
		public Shield(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Shield;
		}
	}

	public class Totem : Buff
	{
		public Totem(XYPosition initPos, int radius) : base(initPos, radius) { }

		public override PropType GetPropType()
		{
			return PropType.Totem;
		}
	}
}

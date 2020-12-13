using System;
using System.Collections.Generic;
using System.Text;
using THUnity2D;

namespace THUnity2D
{
	public enum PropType
	{
		empty = 0,
		p1 = 1,
		p2 = 2
	}
	public class Prop : Obj
	{
		//道具类，未完成
		public readonly PropType propType;
		public Prop(XYPosition initPos, int radius, PropType propType) : base(initPos, radius, false, 0, ObjType.prop)
        {
			this.propType = propType;
        }
	}
}

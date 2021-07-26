using THUnity2D.Interfaces;
using THUnity2D.Utility;

/// <summary>
/// 墙类
/// </summary>

namespace THUnity2D.ObjClasses
{
	public class Wall : Obj
	{
		public Wall(XYPosition initPos, int radius) : base(initPos, radius, ObjType.Wall, ShapeType.Square) { }
		public override bool IsRigid => true;
	}
}

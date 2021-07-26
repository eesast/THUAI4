using THUnity2D.Interfaces;
using THUnity2D.Utility;

/// <summary>
/// 出生点类
/// </summary>

namespace THUnity2D.ObjClasses
{
	public class BirthPoint : Obj
	{
		public BirthPoint(XYPosition initPos, int radius) : base(initPos, radius, ObjType.BirthPoint, ShapeType.Circle) { }
		public override bool IsRigid => true;
	}
}

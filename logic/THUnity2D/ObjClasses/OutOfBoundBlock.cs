using THUnity2D.Interfaces;
using THUnity2D.Utility;

namespace THUnity2D.ObjClasses
{
	public class OutOfBoundBlock : Obj, IOutOfBoundBlock
	{
		public OutOfBoundBlock(XYPosition initPos) : base(initPos, int.MaxValue, ObjType.OutOfBoundBlock, ShapeType.Square) { }

		public override bool IsRigid => true;
	}
}

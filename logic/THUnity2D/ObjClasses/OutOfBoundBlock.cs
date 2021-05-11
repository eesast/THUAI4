
namespace THUnity2D
{
	public class OutOfBoundBlock : Obj
	{
		public OutOfBoundBlock(XYPosition initPos) : base(initPos, int.MaxValue, ObjType.OutOfBoundBlock, ShapeType.Square) { }

		public override bool IsRigid => true;
	}
}

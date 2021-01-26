/// <summary>
/// 出生点类
/// </summary>

namespace THUnity2D
{
	class BirthPoint : Obj
	{
		public BirthPoint(XYPosition initPos, int radius) : base(initPos, radius, true, 0, ObjType.birthPoint) { }
	}
}

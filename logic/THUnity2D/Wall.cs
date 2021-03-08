/// <summary>
/// 墙类
/// </summary>

namespace THUnity2D
{
	public class Wall : Obj
	{
		public Wall(XYPosition initPos, int radius) : base(initPos, radius, true, 0, ObjType.Wall, ShapeType.Sqare) { }
	}
}

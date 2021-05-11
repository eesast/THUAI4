namespace THUnity2D
{

	public enum ObjType
	{
		Empty = 0,
		Wall = 1,
		Prop = 2,
		Bullet = 3,
		BirthPoint = 4,
		OutOfBoundBlock = 5
	}

	public interface IObj : IGameObj
	{
		ObjType ObjType { get; }
		Character? Parent { get; set; }
	}
}

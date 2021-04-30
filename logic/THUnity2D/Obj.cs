//所有游戏人物，道具，子弹的基类
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

	public abstract class Obj : GameObject	//道具，墙
	{
		public override GameObjType GetGameObjType()
		{
			return GameObjType.Obj;
		}
		public readonly ObjType objType;

		private Character? parent = null;		//道具的主人
		public Character? Parent
		{
			get => parent;
			set
			{
				//Operations.Add
				lock (gameObjLock)
				{
					string debugStr = (value == null ? 
					" has been throwed by " + (parent == null ? "null." : parent.ToString()) 
					: "has been picked by " + (value == null ? "null." : value.ToString()));
					parent = value;
					Debug(this, debugStr);
				}
			}
		}

		public Obj(XYPosition initPos, int radius, bool isRigid, int moveSpeed, ObjType objType, ShapeType shape) : base(initPos, radius, isRigid, moveSpeed, shape)
		{
			this.objType = objType;

			Debug(this, " constructed!");
		}

		public override string ToString()
		{
			return objType + "->" + ID + ": " + Position.ToString() + " ";
		}
	}
}

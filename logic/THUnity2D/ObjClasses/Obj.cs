using THUnity2D.Interfaces;
using THUnity2D.Utility;

//所有游戏人物，道具，子弹的基类
namespace THUnity2D.ObjClasses
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

	public abstract class Obj : GameObject, IObj	//道具，墙
	{
		public override GameObjType GetGameObjType()
		{
			return GameObjType.Obj;
		}
		protected ObjType objType;
		public ObjType ObjType => objType;

		private ICharacter? parent = null;		//道具的主人
		public ICharacter? Parent
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
					Debugger.Output(this, debugStr);
				}
			}
		}

		public Obj(XYPosition initPos, int radius, ObjType objType, ShapeType shape) : base(initPos, radius, shape)
		{
			this.objType = objType;

			Debugger.Output(this, " constructed!");
		}

		public override string ToString()
		{
			return objType + "->" + ID + ": " + Position.ToString() + " ";
		}
	}
}

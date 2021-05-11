//所有游戏人物，道具，子弹的基类
namespace THUnity2D
{

	public abstract class Obj : GameObject, IObj	//道具，墙
	{
		public override GameObjType GetGameObjType()
		{
			return GameObjType.Obj;
		}
		protected ObjType objType;
		public ObjType ObjType => objType;

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

		public Obj(XYPosition initPos, int radius, ObjType objType, ShapeType shape) : base(initPos, radius, shape)
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

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Security.Cryptography.X509Certificates;
using System.Text;
//using Commmunication.proto;//通信组定义结构体
//所有游戏人物，道具，子弹的基类
namespace THUnity2D
{

	//参见protobuf

	public enum ObjType
	{
		empty = 0,
		wall = 1,
		prop = 2,
		bullet = 3
	}

	public class Obj : GameObject	//道具，墙
	{
		public readonly ObjType objType;		//通信组实现

		private Character? _parent = null;		//道具的主人
		public Character Parent
        {
			get { return _parent; }
            set
            {
				Operations.Add
					(
						() =>
						{
							string debugStr = (value == null ? 
							" has been throwed by " + (_parent == null ? "null." : _parent.ToString()) 
							: "has been picked by " + (value == null ? "null." : value.ToString()));
							_parent = value;
							Debug(this, debugStr);
						}
					);
            }
        }

		public Obj(XYPosition initPos, int radius, bool isRigid, int moveSpeed, ObjType objType) : base(initPos, radius, isRigid, moveSpeed)
		{
			this.objType = objType;
		}

		public override string ToString()
		{
			return objType + ": " + ID + ", " + Position.ToString() + " ";
		}
	}
}

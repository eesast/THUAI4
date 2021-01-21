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
		public override GameObjType GetGameObjType()
		{
			return GameObjType.obj;
		}
		public readonly ObjType objType;		//通信组实现

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

		public Obj(XYPosition initPos, int radius, bool isRigid, int moveSpeed, ObjType objType) : base(initPos, radius, isRigid, moveSpeed)
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

using System;
using System.Collections.Generic;
using System.Text;
using Communication.Proto;
using THUnity2D;
using GameEngine;

namespace Logic.Server
{
	static class CopyInfo		//把游戏对象的信息转换成用于通信的信息
	{
		public static GameObjInfo Auto(GameObject gameObj)		//自动判断转换的类型
		{
			if (gameObj.GetGameObjType() == THUnity2D.GameObject.GameObjType.Character) return Player((Character)gameObj);
			switch (((Obj)gameObj).objType)
			{
			case THUnity2D.ObjType.Bullet: return Bullet((Bullet)gameObj);
			case THUnity2D.ObjType.Prop: return Prop((Prop)gameObj);
			case THUnity2D.ObjType.Wall: return Wall((Wall)gameObj);
			case THUnity2D.ObjType.BirthPoint: return BirthPoint((BirthPoint)gameObj);
			default:		//错误情况，理论上不应该出现，为了过编译而设置
			{
				GameObjInfo ret = new GameObjInfo();
				ret.GameObjType = GameObjType.OutOfBoundBlock;
				Basic(gameObj, ref ret);
				return ret;
			}
			}
		}

		public static GameObjInfo Player(Character player)		//转换成玩家
		{
			GameObjInfo ret = new GameObjInfo();
			ret.GameObjType = Communication.Proto.GameObjType.Character;
			Basic(player, ref ret);

			ret.TeamID = player.TeamID;
			ret.Ap = player.AP;
			ret.BulletType = ConvertTool.ToCommunicationBulletType(player.bulletType);

			Prop? holdProp = player.HoldProp;		// 防止判断后被突然置null
			ret.PropType = holdProp == null ? Communication.Proto.PropType.Null : ConvertTool.ToCommunicationPropType(holdProp.GetPropType());

			ret.IsDying = player.IsResetting;
			ret.JobType = ConvertTool.ToCommunicationJobType(player.jobType);
			ret.CD = player.CD;
			ret.MaxBulletNum = player.MaxBulletNum;
			ret.BulletNum = player.BulletNum;
			ret.MaxHp = player.MaxHp;
			ret.Hp = player.HP;
			ret.LifeNum = player.LifeNum;

			return ret;
		}

		public static GameObjInfo Bullet(Bullet bullet)		//转换成子弹
		{
			GameObjInfo ret = new GameObjInfo();
			ret.GameObjType = Communication.Proto.GameObjType.Bullet;
			Basic(bullet, ref ret);

			ret.TeamID = bullet.Parent == null ? Team.invalidTeamID : bullet.Parent.TeamID;
			ret.Ap = bullet.AP;
			ret.BulletType = ConvertTool.ToCommunicationBulletType(bullet.BulletType);

			return ret;
		}

		public static GameObjInfo Prop(Prop prop)			//转换成道具
		{
			GameObjInfo ret = new GameObjInfo();
			ret.GameObjType = Communication.Proto.GameObjType.Prop;
			Basic(prop, ref ret);

			ret.PropType = ConvertTool.ToCommunicationPropType(prop.GetPropType());
			ret.IsLaid = prop.Laid;
			ret.TeamID = prop.Parent == null ? Team.invalidTeamID : prop.Parent.TeamID;

			return ret;
		}

		public static GameObjInfo Wall(Wall wall)			//转换成墙
		{
			GameObjInfo ret = new GameObjInfo();
			ret.GameObjType = Communication.Proto.GameObjType.Wall;
			Basic(wall, ref ret);

			return ret;
		}

		public static GameObjInfo BirthPoint(BirthPoint birthPoint)		//转换成出生点
		{
			GameObjInfo ret = new GameObjInfo();
			ret.GameObjType = Communication.Proto.GameObjType.BirthPoint;
			Basic(birthPoint, ref ret);

			return ret;
		}

		private static void Basic(GameObject gameObj, ref GameObjInfo ret)		//拷贝基本信息
		{
			ret.Guid = gameObj.ID;
			ret.X = gameObj.Position.x;
			ret.Y = gameObj.Position.y;
			ret.FacingDirection = gameObj.FacingDirection;
			ret.MoveSpeed = gameObj.MoveSpeed;
			ret.CanMove = gameObj.CanMove;
			ret.IsMoving = gameObj.IsMoving;
			ret.ShapeType = ConvertTool.ToCommunicationShapeType(gameObj.Shape);
			ret.Radius = gameObj.Radius;
		}
	}
}

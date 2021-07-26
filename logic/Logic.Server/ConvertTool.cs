using System;

namespace Logic.Server
{
	public static class ConvertTool			//用于通信所用类型与游戏内部所用类型之间的转换
	{
		private static D AllEnums<S, D>(S src, D defaultVal)    //应用反射转换枚举成员名相同的枚举类型，若转换失败则使用默认值defaultVal
			where S : notnull
			where D : notnull
		{
			try
			{
				return (D)Enum.Parse(typeof(D), src.ToString() ?? "");
			}
			catch
			{
				return defaultVal;
			}
		}
		public static THUnity2D.ObjClasses.JobType ToGameJobType(Communication.Proto.JobType jobType)
		{
			return AllEnums(jobType, THUnity2D.ObjClasses.JobType.InvalidJobType);
		}

		public static Communication.Proto.JobType ToCommunicationJobType(THUnity2D.ObjClasses.JobType jobType)
		{
			return AllEnums(jobType, Communication.Proto.JobType.Job0);
		}

		public static THUnity2D.ObjClasses.PropType ToGamePropType(Communication.Proto.PropType propType)
		{
			return AllEnums(propType, THUnity2D.ObjClasses.PropType.Null);
		}

		public static Communication.Proto.PropType ToCommunicationPropType(THUnity2D.ObjClasses.PropType jobType)
		{
			return AllEnums(jobType, Communication.Proto.PropType.Null);
		}

		public static Communication.Proto.ShapeType ToCommunicationShapeType(THUnity2D.Interfaces.ShapeType shapeType)
		{
			return AllEnums(shapeType, Communication.Proto.ShapeType.Circle);
		}

		public static Communication.Proto.BulletType ToCommunicationBulletType(THUnity2D.ObjClasses.BulletType bulletType)
		{
			return AllEnums(bulletType, Communication.Proto.BulletType.Bullet0);
		}

		public static Communication.Proto.ColorType ToCommunicationColorType(Gaming.Map.ColorType colorType)
		{
			return AllEnums(colorType, Communication.Proto.ColorType.None);
		}
	}
}

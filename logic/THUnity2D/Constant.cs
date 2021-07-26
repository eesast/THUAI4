using THUnity2D.ObjClasses;
using THUnity2D.Utility;

namespace THUnity2D
{
	public static class Constant
	{
		public const int numOfGridPerCell = 1000;               //每个的坐标单位数
		public const int numOfStepPerSecond = 20;               //每秒行走的步数
		public const int addScoreWhenKillOnePlayer = 0;         //击杀一名玩家后的加分
		public const int producePropTimeInterval = 15 * 1000;   //产生道具时间间隔（毫秒）

		public const int basicPlayerMoveSpeed = numOfGridPerCell * 4;
		public const int basicBulletMoveSpeed = numOfGridPerCell * 6;
		public const int thrownPropMoveSpeed = numOfGridPerCell * 8;

		public const int objMaxRadius = numOfGridPerCell / 2;
		public const int playerRadius = objMaxRadius;
		public const int wallRadius = objMaxRadius;
		public const int birthPointRadius = objMaxRadius;
		public const int bulletRadius = objMaxRadius / 2;
		public const int unpickedPropRadius = objMaxRadius;

		public const int buffPropTime = 30 * 1000;          //buff道具持续时间（毫秒）
		public const int shieldTime = buffPropTime;
		public const int totemTime = buffPropTime;
		public const int spearTime = buffPropTime;

		public const double bikeMoveSpeedBuff = 1.5;        //道具效果
		public const double amplifierAtkBuff = 2.0;
		public const double jinKeLaCdDiscount = 0.25;
		public const int riceHpAdd = Character.basicHp / 2;

		public const int mineTime = 60 * 1000;              //地雷埋藏的持续时间
		public const double dirtMoveSpeedDebuff = 1.0 / bikeMoveSpeedBuff;
		public const double attenuatorAtkDebuff = 1.0 / amplifierAtkBuff;
		public const double dividerCdDiscount = 4.0;

		public const int deadRestoreTime = 30 * 1000;       //死亡恢复时间
		public const int shieldTimeAtBirth = 5 * 1000;      //出生时的盾牌时间

		public static XYPosition CellToGrid(int x, int y)   //求格子的中心坐标
		{
			XYPosition ret = new XYPosition(x * numOfGridPerCell + numOfGridPerCell / 2,
				y * numOfGridPerCell + numOfGridPerCell / 2);
			return ret;
		}
		public static int GridToCellX(XYPosition pos)       //求坐标所在的格子的x坐标
		{
			return pos.x / numOfGridPerCell;
		}
		public static int GridToCellY(XYPosition pos)      //求坐标所在的格子的y坐标
		{
			return pos.y / numOfGridPerCell;
		}
	}
}

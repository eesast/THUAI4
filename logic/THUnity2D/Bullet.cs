using System;
using System.Collections.Generic;
using System.Diagnostics.SymbolStore;
using System.Dynamic;
using System.Text;
using System.Threading;

namespace THUnity2D
{
	public enum BulletType
	{
		Bullet0 = 0,
		Bullet1 = 1,
		Bullet2 = 2,
		Bullet3 = 3,
		Bullet4 = 4,
		Bullet5 = 5,
		Bullet6 = 6
	}
	public sealed class Bullet : Obj
	{
		private int ap;     //攻击力
		public int AP => ap;

		private readonly bool hasSpear;
		public bool HasSpear => hasSpear;

		private readonly BulletType bulletType;
		public BulletType BulletType => bulletType;

		public Bullet(XYPosition initPos, int radius, int basicMoveSpeed, BulletType bulletType, int ap, bool hasSpear) 
			: base(initPos, radius, false, basicMoveSpeed, ObjType.Bullet, ShapeType.Circle)
		{
			this.ap = ap;
			this.hasSpear = hasSpear;
			this.bulletType = bulletType;
			switch (bulletType)
			{
			default:
			case BulletType.Bullet0:
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
				IsRigid = true;
				break;
			case BulletType.Bullet1:
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 2;
				IsRigid = false;
				break;
			case BulletType.Bullet2:
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed / 2;
				IsRigid = true;
				break;
			case BulletType.Bullet3:
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed / 2;
				IsRigid = true;
				break;
			case BulletType.Bullet4:
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed * 3;
				IsRigid = false;
				break;
			case BulletType.Bullet5:
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
				IsRigid = true;
				break;
			case BulletType.Bullet6:
				MoveSpeed = OrgMoveSpeed = basicMoveSpeed;
				IsRigid = true;
				break;
			}
		}

		//返回染色范围，相对自己的相对距离
		public XYPosition[] GetColorRange()
		{
			XYPosition[] range;
			switch (bulletType)
			{
			case BulletType.Bullet0:
			case BulletType.Bullet1:
			case BulletType.Bullet5:
				range = new XYPosition[9];
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						range[i * 3 + j].x = i - 1;
						range[i * 3 + j].y = j - 1;
					}
				}
				break;
			case BulletType.Bullet2:
				range = new XYPosition[49];
				for (int i = 0; i < 5; ++i)
				{
					for (int j = 0; j < 5; ++j)
					{
						range[i * 5 + j].x = i - 2;
						range[i * 5 + j].y = j - 2;
					}
				}
				break;
			case BulletType.Bullet3:
				range = new XYPosition[25];
				for (int i = 0; i < 5; ++i)
				{
					for (int j = 0; j < 5; ++j)
					{
						range[i * 5 + j].x = i - 2;
						range[i * 5 + j].y = j - 2;
					}
				}
				break;
			case BulletType.Bullet4:
			case BulletType.Bullet6:
				range = new XYPosition[1];
				range[0].x = range[0].y = 0;
				break;
			default:
				range = new XYPosition[0];
				break;
			}
			return range;
		}

		//返回爆炸范围，相对自己的相对距离
		public XYPosition[] GetAttackRange()
		{
			XYPosition[] range;
			switch (bulletType)
			{
			case BulletType.Bullet0:
			case BulletType.Bullet1:
			case BulletType.Bullet5:
			case BulletType.Bullet6:
				range = new XYPosition[9];
				for (int i = 0; i < 3; ++i)
				{
					for (int j = 0; j < 3; ++j)
					{
						range[i * 3 + j].x = i - 1;
						range[i * 3 + j].y = j - 1;
					}
				}
				break;
			case BulletType.Bullet2:
				range = new XYPosition[49];
				for (int i = 0; i < 7; ++i)
				{
					for (int j = 0; j < 7; ++j)
					{
						range[i * 7 + j].x = i - 3;
						range[i * 7 + j].y = j - 3;
					}
				}
				break;
			case BulletType.Bullet4:
				range = new XYPosition[49];
				for (int i = 0; i < 7; ++i)
				{
					for (int j = 0; j < 7; ++j)
					{
						range[i * 7 + j].x = i - 3;
						range[i * 7 + j].y = j - 3;
					}
				}
				break;
			case BulletType.Bullet3:
				range = new XYPosition[1];
				range[0].x = range[0].y = 0;
				break;
			default:
				range = new XYPosition[0];
				break;
			}
			return range;
		}
	}
}

using System;
using System.Collections.Generic;
using System.Text;

namespace THUnity2D
{
	public interface IMovable : IGameObj
	{
		public int MoveSpeed { get; }

		public long Move(Vector moveVec);

		protected bool IgnoreCollide(IGameObj targetObj);

		/// <summary>
		/// 检查下一步位于nextPos时是否会与targetObj碰撞
		/// </summary>
		/// <param name="targetObj">被动碰撞物</param>
		/// <param name="nextPos">obj下一步想走的位置</param>
		/// <returns>如果会碰撞，返回true</returns>
		public bool WillCollideWith(IGameObj targetObj, XYPosition nextPos)
		{
			if (!targetObj.IsRigid || targetObj.ID == ID) return false; //不检查自己和非刚体

			if (IgnoreCollide(targetObj)) return false;

			int deltaX = Math.Abs(nextPos.x - targetObj.Position.x), deltaY = Math.Abs(nextPos.y - targetObj.Position.y);

			//默认obj是圆形的，因为能移动的物体目前只有圆形（会移动的道具尚未被捡起，其形状没有意义，可默认为圆形）

			switch (targetObj.Shape)
			{
				case ShapeType.Circle:       //圆与圆碰撞
					{
						return (long)deltaX * deltaX + (long)deltaY * deltaY < ((long)Radius + targetObj.Radius) * ((long)Radius + targetObj.Radius);
					}
				case ShapeType.Square:        //圆与正方形碰撞
					{
						if (deltaX >= targetObj.Radius + Radius || deltaY >= targetObj.Radius + Radius) return false;
						if (deltaX < targetObj.Radius || deltaY < targetObj.Radius) return true;
						return (long)(deltaX - targetObj.Radius) * (deltaX - targetObj.Radius) + (long)(deltaY - targetObj.Radius) * (deltaY - targetObj.Radius) < (long)Radius * (long)Radius;
					}
			}
			return false;
		}


	}
}

using System;

/// <summary>
/// 游戏对象的公共基类GameObject
/// </summary>

namespace THUnity2D
{
	public abstract class GameObject
	{
		public enum GameObjType
		{
			Character = 0,
			Obj = 1
		}
		public enum ShapeType
		{
			Null = 0,
			Circle = 1,
			Square = 2
		}
		public abstract GameObjType GetGameObjType();	//给C++/CLI调试用的，因为我不知道C++/CLI怎么用is操作符（狗头保命）

		//public const int MinSpeed = 1;                  //最小速度
		//public const int MaxSpeed = int.MaxValue;  //最大速度

		protected readonly object gameObjLock = new object();
		protected readonly object moveLock = new object();
		public object MoveLock => moveLock;

		private static long currentMaxID = 0;           //目前游戏对象的最大ID
		public const long invalidID = long.MaxValue;            //无效的ID
		public const long noneID = long.MinValue;				//不存在ID
		public long ID { get; }							//ID
		
		private XYPosition position;		//位置
		public XYPosition Position { get => position; protected set { lock (gameObjLock) { position = value; } } }
		public readonly XYPosition orgPos;

		//Direction
		private double facingDirection = 0.0;		//面向的方向，去极角的弧度值
		public double FacingDirection
		{
			get => facingDirection;
			private set
			{
				//Operations.Add
				lock (gameObjLock)
				{
					facingDirection = value;
				}
			}
		}

		public abstract bool IsRigid { get; }     //是否是刚体，即是否具有碰撞

		protected ShapeType shape;
		public ShapeType Shape => shape;				//形状

		//当前是否能移动

		private bool canMove = false;              //当前是否能移动

		public bool CanMove
		{
			get => canMove;
			set
			{
				//Operations.Add
				lock (gameObjLock)
				{
					canMove = value;
					Debug(this, canMove ? "Enable move!" : "Disable move!");
				}
			}
		}

		//当前是否正在移动
		private bool isMoving = false;
		public bool IsMoving
		{
			get => isMoving;
			set
			{
				//Operations.Add
				lock (gameObjLock)
				{
					isMoving = value;
					Debug(this, isMoving ? " begin to move!" : " end moving!");
				}
			}
		}

		private bool isResetting = false;
		public bool IsResetting
		{
			get => isResetting;
			set
			{
				lock (gameObjLock)
				{
					isResetting = value;
				}
			}
		}

		public bool IsAvailable => !IsMoving && CanMove && !IsResetting;    //是否能接收指令


		//移动，改变坐标，反馈实际走的长度的平方
		protected long Move(Vector displacement)
		{
			var deltaPos = Vector.Vector2XY(displacement);
			//Operations.Add
			lock (gameObjLock)
			{
				FacingDirection = displacement.angle;
				this.position.x += deltaPos.x;
				this.position.y += deltaPos.y;
			}
			return deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y;
		}

		//圆或内切圆半径
		private int radius;
		public int Radius => radius;

		public virtual void Reset()
		{
			lock (moveLock)
			{
				this.position = orgPos;
				facingDirection = 0.0;
				isMoving = false;
				canMove = false;
			}
		}

		public GameObject(XYPosition initPos, int radius, ShapeType shape)
		{
			ID = currentMaxID;
			++currentMaxID;

			this.position = initPos;
			this.orgPos = initPos;
			this.radius = radius;
			this.shape = shape;
		}

		public override string ToString()
		{
			return ID.ToString() + ": " + Position.ToString();
		}

		//用于Debug时从控制台观察到各个游戏对象的状况
		public static void Debug(object current, string str)
		{

#if DEBUG
			Console.WriteLine(current.GetType() + " " + current.ToString() + str);
#endif

		}


		/// <summary>
		/// 检查下一步位于nextPos时是否会与targetObj碰撞
		/// </summary>
		/// <param name="targetObj">被动碰撞物</param>
		/// <param name="nextPos">obj下一步想走的位置</param>
		/// <returns>如果会碰撞，返回true</returns>
		public virtual bool WillCollideWith(GameObject targetObj, XYPosition nextPos)
		{
			if (!targetObj.IsRigid || targetObj.ID == ID) return false; //不检查自己和非刚体

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

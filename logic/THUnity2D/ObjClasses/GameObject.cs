using System;
using THUnity2D.Interfaces;
using THUnity2D.Utility;

/// <summary>
/// 游戏对象的公共基类GameObject
/// </summary>

namespace THUnity2D.ObjClasses
{

	public enum GameObjType
	{
		Character = 0,
		Obj = 1
	}

	public abstract class GameObject : IGameObj
	{
		
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
					Debugger.Output(this, canMove ? "Enable move!" : "Disable move!");
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
					Debugger.Output(this, isMoving ? " begin to move!" : " end moving!");
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
		public long Move(Vector displacement)
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
	}
}

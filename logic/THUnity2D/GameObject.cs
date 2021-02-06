using System;
using System.Collections.Concurrent;

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
			Sqare = 2
		}
		public abstract GameObjType GetGameObjType();	//给C++/CLI调试用的，因为我不知道C++/CLI怎么用is操作符（狗头保命）

		public const int MinSpeed = 1;                  //最小速度
		public const int MaxSpeed = int.MaxValue;  //最大速度

		protected readonly object gameObjLock = new object();
		public readonly object moveLock = new object();

		private static long currentMaxID = 0;           //目前游戏对象的最大ID
		public const long invalidID = long.MaxValue;            //无效的ID
		public const long noneID = long.MinValue;				//不存在ID
		public long ID { get; }							//ID
		
		private XYPosition position;		//位置
		public XYPosition Position { get => position; }
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

		public bool IsRigid { get; protected set; }     //是否是刚体，即是否具有碰撞

		private readonly ShapeType shape;
		public ShapeType Shape { get => shape; }		//形状

		private int _moveSpeed;
		public int MoveSpeed
        {
			get => _moveSpeed;
			protected set
			{
				lock (gameObjLock)
				{
					_moveSpeed = value;
				}
			}
		}
		public readonly int orgMoveSpeed;

		//当前是否能移动

		private bool canMove = false;              //当前是否能移动

		public bool CanMove
        {
			get { return canMove; }
			set
			{
				//Operations.Add
				lock(gameObjLock)
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

		public bool IsAvailable { get => !IsMoving && CanMove && !IsResetting; }    //是否能接收指令


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
		public int Radius
		{
			get => radius;
		}

		public virtual void Reset()
		{
			lock (moveLock)
			{
				this.position = orgPos;
				_moveSpeed = orgMoveSpeed;
				facingDirection = 0.0;
				isMoving = false;
				canMove = false;
			}
		}

		public GameObject(XYPosition initPos, int radius, bool isRigid, int moveSpeed, ShapeType shape)
		{
			ID = currentMaxID;
			++currentMaxID;

			this.position = initPos;
			this.orgPos = initPos;
			this.radius = radius;
			this.IsRigid = isRigid;
			this._moveSpeed = Math.Min(Math.Max(moveSpeed, MinSpeed), MaxSpeed);
			this.orgMoveSpeed = this._moveSpeed;
			this.shape = shape;
		}

		public override string ToString()
		{
			return ID.ToString() + ": " + Position.ToString();
		}

		//用于Debug时从控制台观察到各个游戏对象的状况
		public static void Debug(GameObject current, string str)
		{

#if DEBUG
			Console.WriteLine(current.GetType() + " " + current.ToString() + str);
#endif

		}


	}
}

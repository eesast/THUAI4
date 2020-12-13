using System;
using System.Collections.Concurrent;

/// <summary>
/// 游戏对象的公共基类GameObject
/// </summary>

namespace THUnity2D
{
	public abstract class GameObject
	{
		public const int unitPerCell = 1024;			//一个大格含有的坐标个数

		public const int MinSpeed = 1;                  //最小速度
		public const int MaxSpeed = (int)(unitPerCell * 0.414);	//最大速度

		private static long currentMaxID = 0;			//目前游戏对象的最大ID
		public long ID { get; }							//ID
		
		protected readonly BlockingCollection<Action> Operations = new BlockingCollection<Action>();//事件队列

		private XYPosition _position;		//位置
		public XYPosition Position { get { return _position; } }

		//Direction
		private double _facingDirection = 0.0;		//面向的方向，去极角的弧度值
		public double FacingDirection
        {
			get => _facingDirection;
			private set
            {
				Operations.Add
					(
						() =>
                        {
							_facingDirection = value;
                        }
					);
            }
        }

		private bool _canMove = false;				//当前是否能移动
		
		public bool IsRigid { get; protected set; }     //是否是刚体，即是否具有碰撞

		private int _moveSpeed;
		public int MoveSpeed
        {
			get => _moveSpeed;
			set
            {
				Operations.Add
					(
						() =>
                        {
							//保证速度在MinSpeed与MaxSpeed之间
							_moveSpeed = Math.Min(Math.Max(value, MinSpeed), MaxSpeed);
							Debug(this, ", the speed of which has been set to " + _moveSpeed.ToString());
						}
					);
			}
        }

		//当前是否能移动
		public bool CanMove
        {
			get { return _canMove; }
			set
			{
				Operations.Add
					(
						() =>
						{
							_canMove = value;
							Debug(this, _canMove ? "Enable move!" : "Disable move!");
						}
					);
            }
		}

		//移动，改变坐标
		public void Move(Vector displacement)
        {
			Operations.Add
				(
					() =>
                    {
						var deltaPos = Vector.Vector2XY(displacement);
						this._position.x += deltaPos.x;
						this._position.y += deltaPos.y;
						FacingDirection = displacement.angle;
                    }
				);
        }

		//物体半径
		private int _radius;
		public int Radius
		{
			get => _radius;
		}

		public GameObject(XYPosition initPos, int radius, bool isRigid, int moveSpeed)
		{
			ID = currentMaxID;
			++currentMaxID;

			this._position = initPos;
			this._radius = radius;
			this.IsRigid = isRigid;
			this._moveSpeed = Math.Min(Math.Max(moveSpeed, MinSpeed), MaxSpeed);

			new System.Threading.Thread(	//开辟一个线程，不断取出里面的待办事项进行办理
				() =>
				{
					while (true)
					{
						Operations.Take()();
					}
				}
			)
			{ IsBackground = true }.Start();

			Console.WriteLine("new gameobject constructed");//debug
		}

		//用于Debug时从控制台观察到各个游戏对象的状况
		public static void Debug(GameObject current, string str)
		{
			Console.WriteLine(current.GetType() + " " + current.ToString() + str);
		}


	}
}

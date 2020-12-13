
/// <summary>
/// 储存被暂时删除的代码，
/// 日后用到了来这里找
/// </summary>

////////////////////////////////
//
//XYPosition.cs
//

//public static XYPosition operator +(XYPosition a, XYPosition b)
//{
//    return new XYPosition(a.x + b.x, a.y + b.y);
//}
//public static XYPosition operator -(XYPosition a, XYPosition b)
//{
//    return new XYPosition(a.x - b.x, a.y - b.y);
//}
//public static XYPosition operator *(XYPosition a, int b)
//{
//    return new XYPosition(a.x * b, a.y * b);
//}
//public static XYPosition operator *(int b, XYPosition a)
//{
//    return new XYPosition(a.x * b, a.y * b);
//}
//public static bool operator ==(XYPosition xy1, XYPosition xy2)
//{
//    return xy1.x == xy2.x && xy1.y == xy2.y;
//}
//public static bool operator !=(XYPosition xy1, XYPosition xy2)
//{
//    return !(xy1 == xy2);
//}
//public static bool IntEqual(XYPosition xy1, XYPosition xy2)
//{
//    return (int)xy1.x == (int)xy2.x && (int)xy1.y == (int)xy2.y;
//}
//public XYPosition GetMid()
//{
//    return new XYPosition((int)x + 0.5, (int)y + 0.5);
//}
//public static double Distance(XYPosition position1, XYPosition position2)
//{
//    return Math.Sqrt(Math.Pow(Math.Abs(position1.x - position2.x), 2) + Math.Pow(Math.Abs(position1.y - position2.y), 2));
//}
//public static double ManhattanDistance(XYPosition position1, XYPosition position2)
//{
//    return Math.Abs(position1.x - position2.x) + Math.Abs(position1.y - position2.y);
//}

////////////////////////////
//
// point.cs
//

//namespace THUnity2D
//{
//    public class Point//从坐标转化为地图内节点
//    {
//        public readonly int x;
//        public readonly int y;
//        public Point(XYPosition loc) {
//            x = (int) loc.x / 20;
//            y =  (int) loc.y / 20;
//        }
//        public Point() {
//            x = 0;
//            y = 0;
//        }
//        public override string ToString()
//        {

//            return "(" + x + "," + y + ")";
//        }
//    }
//}

//////////////////////////
//
// Vector.cs
//

//public static Vector operator *(Vector vector, double number)
//{
//	return new Vector(vector.angle, vector.length * number);
//}
//public static Vector operator *(double number, Vector vector)
//{
//	return new Vector(vector.angle, vector.length * number);
//}
//public static bool operator ==(Vector vector1, Vector vector2)
//{
//	return Math.Abs(vector1.angle - vector2.angle) < 1e-8 && Math.Abs(vector1.length - vector2.length) < 1e-8;
//}
//public static bool operator !=(Vector vector1, Vector vector2)
//{
//	return !(vector1 == vector2);
//}

////////////////////////////////
//
// GameObject.cs
//

////父对象
//protected delegate void ParentDeleteHandler();
//protected event ParentDeleteHandler? OnParentDelete;
//protected delegate void ParentAddHandler();
//protected event ParentAddHandler? OnParentAdd;
//private void AddParent(GameObject parent)
//{
//	this._parent = parent;
//	// this._parent.OnChildrenAdded(this);
//	OnParentAdd?.Invoke();
//}
//private GameObject? _parent;
//public GameObject? Parent
//{
//	get { return _parent; }
//	set
//	{
//		Operations.Add(() => {//需要找一找operation是在哪处理的。。。
//			AddParent(value);
//		});
//	}
//}

//-------------------------------
////Children  
//public ConcurrentDictionary<GameObject, byte> ChildrenGameObjectList { get; } = new ConcurrentDictionary<GameObject, byte>();

////Children end

//protected internal Point point = new Point();
////没有实现委托，

//-------------------------------
//public XYPosition Position
//{
//	get { return _position; }
//	set
//	{
//		_position = value;//调试
//		point = new Point(value);
//		Operations.Add(
//			() => {
//				Debug(this, "Prompt to change position to : " + value.ToString());

//			}
//			);
//	}
//}

//-----------------------------------
//public double FacingDirection
//{
//	get => this._facingDirection;
//	set { Operations.Add(() => { this._facingDirection = value; DirectionChanged?.Invoke(this, _facingDirection); }); }
//}

//----------------------
//public delegate void DirectionChangedHandler(GameObject sender, double direction);
//public event DirectionChangedHandler? DirectionChanged; // 声明事件
//direction end

//---------------------------------------------
//protected double _frameRate = 30;
//public double FrameRate
//{
//	get => _frameRate;
//	set
//	{
//		Operations.Add(
//			() =>
//					//lock (privateLock)
//					{
//				if (value < 1)//限制帧率最小为每秒一帧
//							this._frameRate = 1;
//				else if (value > 100)//限制帧率最大为每秒100帧
//							this._frameRate = 100;
//				else
//					this._frameRate = value;
//				foreach (var item in ChildrenGameObjectList.Keys)
//					item.FrameRate = this._frameRate;
//				return;
//			});
//	}
//}

//---------------------------------------------

//System.Threading.Semaphore canMoveSema = new System.Threading.Semaphore(0, 1);
//private System.Threading.Thread? _movingThread;
//protected Vector _velocity = new Vector();
//protected object _velocityLock = new object();
//public Vector Velocity
//{
//	get => _velocity;
//	set
//	{
//		if (_movingThread == null)
//		{
//			_movingThread = new System.Threading.Thread(
//				() =>
//				{
//					while (true)
//					{
//						while (!canMove)
//						{
//							canMoveSema.WaitOne();
//						}
//						while (canMove)
//						{
//							Move(_velocity.angle, _velocity.length / _frameRate);
//							System.Threading.Thread.Sleep((int)(1000.0 / _frameRate));
//									//}
//								}
//					}
//				});
//			_movingThread.IsBackground = true;
//			_movingThread.Start();
//		}
//		lock (_velocityLock)
//		{
//			if (value.length < MinSpeed)
//			{
//				_velocity = new Vector(value.angle, 0);
//				//MovingTimer.Change(-1, -1);
//				canMove = false;
//				return;
//			}
//			this._velocity = value;
//			canMove = true;
//			try
//			{
//				canMoveSema.Release();
//			}
//			catch (System.Threading.SemaphoreFullException)
//			{ }
//			//}
//		}
//	}
//}
////velocity end



////Movable
//private bool _movable;
//public bool Movable
//{
//	get => this._movable;
//	set { Operations.Add(() => { this._movable = value; }); }
//}
////Movable end

////Move
//public delegate void MoveHandler(GameObject sender, double angle, double distance, XYPosition previousPosition);
//internal event MoveHandler? OnMove;
//public delegate void MoveCompleteHandler(GameObject sender);
//public event MoveCompleteHandler? MoveComplete;
//public delegate void MoveStartHandler(GameObject sender);
//public event MoveStartHandler? MoveStart;
//protected int lastMoveTime = 0;
//public virtual void Move(double angle, double distance)
//{
//	if (double.IsNaN(angle))
//		angle = 0;
//	if (double.IsNaN(distance))
//		distance = 0;
//	if (distance == 0)
//		return;
//	if ((Environment.TickCount - lastMoveTime) < 800.0 / _frameRate)
//		return;
//	lastMoveTime = Environment.TickCount;
//	Operations.Add(
//		() =>
//				//lock (privateLock)
//				{
//			if (!this._movable)
//				return;
//			MoveStart?.Invoke(this);
//			XYPosition previousPosition = new XYPosition(Math.Round(_position.x, 6), Math.Round(_position.y, 6));
//			positiontmp = previousPosition + new XYPosition(distance * Math.Cos(angle), distance * Math.Sin(angle));
//			if (positiontmp.x <= 1000 && positiontmp.y <= 1000 && positiontmp.x >= 0 && positiontmp.y >= 0)
//			{
//				_position = positiontmp;
//				point = new Point(_position);
//			}
//					//Debug(this, "Move from " + previousPosition.ToString() + " angle : " + angle + " distance : " + distance + " aim : " 
//					//   +_position.ToString());
//					OnMove?.Invoke(this, angle, distance, previousPosition);
//			Debug(this, "Now pos : " + this.point.ToString());
//			MoveComplete?.Invoke(this);
//		});
//}
////Move end


//public GameObject(XYPosition position, GameObject? parent = null) : this(parent)
//		{
//	this.Position = position;
//	this.point = new Point(position);
//}

//========================================

//MapCell.cs
//public enum Color
//{ //墙不可染色
//    empty = 0, team1 = 1, team2 = 2, team3 = 3, team4 = 4, wall = 5
//}
//public class MapCell : GameObject
//{//在空中的投掷物无法访问，这是也许是合理的？？没写分层，要是需要到时候再改
//    public readonly object publiclock = new object();
//    protected readonly object privatelock = new object();
//    protected Color _color;
//    protected ConcurrentDictionary<byte, GameObject> objects = new ConcurrentDictionary<byte, GameObject>();//此处的东西,key为0，1，2
//                                                                                                            //key0表示地面上的东西，1表示道具，2表示飞行物, 其他人物等东西不会共占一格
//    public ConcurrentDictionary<byte, GameObject> Objects
//    {
//        get => objects;
//    }
//    public Color color
//    {
//        get => _color;
//        set
//        {
//            Operations.Add(
//            () =>
//            {
//                _color = value;
//            }
//            );
//        }
//    }
//    public MapCell()
//    {
//        _color = Color.empty;
//    }
//    protected void AddGameObject(GameObject obj, byte key)
//    {
//        if (key != 0 && key != 1 && key != 2)
//            return;
//        lock (privatelock)
//        {
//            if (!objects.ContainsKey(key))
//            {
//                objects.TryAdd(key, obj);
//            }
//        }
//    }

//    protected void DeleteGameObject(byte key)
//    {
//        lock (privatelock)
//        {
//            if (!objects.ContainsKey(key))
//                return;
//            GameObject? tmp;

//            if (objects.TryRemove(key, out tmp))
//                Debug(this, "delete succeed");

//        }
//    }

//    protected GameObject GetObject(byte key)
//    {
//        lock (privatelock)
//        {
//            if (!objects.ContainsKey(key))
//            {
//                return null;
//            }
//            return objects[key];
//        }
//    }

//}

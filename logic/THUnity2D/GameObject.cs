using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.ComponentModel;
using System.Diagnostics;
//注：部分实现，功能不完全，
//碰撞逻辑没有实现
//position的想法(double x, double y),限制x，y范围为0-1000， 映射到map的50*50格子中
namespace THUnity2D
{
    public class GameObject//基类部分实现,仅实现委托，具体回调未编写
    {
        private static Int64 currentMaxID = 0;
        public readonly Int64 ID;//ID
        public const double MinSpeed = 0.0001;



        protected readonly BlockingCollection<Action> Operations = new BlockingCollection<Action>();//事件队列

        //父对象
        protected delegate void ParentDeleteHandler();
        protected event ParentDeleteHandler? OnParentDelete;
        protected delegate void ParentAddHandler();
         protected event ParentAddHandler? OnParentAdd;
        private void AddParent(GameObject parent)
        {
            this._parent = parent;
           // this._parent.OnChildrenAdded(this);
            OnParentAdd?.Invoke();
        }
        private GameObject? _parent;
        public GameObject? Parent{
        get {return _parent;}
            set {
                Operations.Add(()=> {//需要找一找operation是在哪处理的。。。
                    AddParent(value);
                });  
            }
         }

        //Children  
        public ConcurrentDictionary<GameObject, byte> ChildrenGameObjectList { get; } = new ConcurrentDictionary<GameObject, byte>();
        //todo，对应的方法未实现

        //Children end


        protected internal Point point= new Point();
        //没有实现委托，




        protected internal XYPosition _position = new XYPosition();
        protected internal XYPosition positiontmp = new XYPosition();
        public XYPosition Position
        {
            get { return _position; }
            set {
                _position = value;//调试
                point =new Point(value);
                Operations.Add(
                    () => {
                        //事件处理，据说事件.Invoke,比直接触发event要高效一些。。。
                        Debug(this, "Prompt to change position to : " + value.ToString());

                    }
                    );            
            }
        }

        //Direction
        protected double _facingDirection = 0;
        public double FacingDirection
        {
            get => this._facingDirection;
            set { Operations.Add(() => { this._facingDirection = value; DirectionChanged?.Invoke(this, _facingDirection); }); }
        }
        public delegate void DirectionChangedHandler(GameObject sender, double direction);
        public event DirectionChangedHandler? DirectionChanged; // 声明事件
                                                                //direction end

        //todo
        //实现速度函数
        //velocity
        protected double _frameRate = 30;
        public double FrameRate
        {
            get => _frameRate;
            set
            {
                Operations.Add(
                    () =>
                    //lock (privateLock)
                    {
                        if (value < 1)//限制帧率最小为每秒一帧
                            this._frameRate = 1;
                        else if (value > 100)//限制帧率最大为每秒100帧
                            this._frameRate = 100;
                        else
                            this._frameRate = value;
                        foreach (var item in ChildrenGameObjectList.Keys)
                            item.FrameRate = this._frameRate;
                        return;
                    });
            }
        }
        bool canMove = false;
        System.Threading.Semaphore canMoveSema = new System.Threading.Semaphore(0, 1);
        private System.Threading.Thread? _movingThread;
        protected Vector _velocity = new Vector();
        protected object _velocityLock = new object();
        public Vector Velocity
        {
            get => _velocity;
            set
            {
                if (_movingThread == null)
                {
                    _movingThread = new System.Threading.Thread(
                        () =>
                        {
                            while (true)
                            {
                                while (!canMove)
                                {
                                    canMoveSema.WaitOne();
                                }
                                while (canMove)
                                {
                                    Move(_velocity.angle, _velocity.length / _frameRate);
                                    System.Threading.Thread.Sleep((int)(1000.0 / _frameRate));
                                    //}
                                }
                            }
                        });
                    _movingThread.IsBackground = true;
                    _movingThread.Start();
                }
                lock (_velocityLock)
                {
                    if (value.length < MinSpeed)
                    {
                        _velocity = new Vector(value.angle, 0);
                        //MovingTimer.Change(-1, -1);
                        canMove = false;
                        return;
                    }
                    this._velocity = value;
                    canMove = true;
                    try
                    {
                        canMoveSema.Release();
                    }
                    catch (System.Threading.SemaphoreFullException)
                    { }
                    //}
                }
            }
        }
        //velocity end



        //Movable
        private bool _movable;
        public bool Movable
        {
            get => this._movable;
            set { Operations.Add(() => { this._movable = value; }); }
        }
        //Movable end

        //Move
        public delegate void MoveHandler(GameObject sender, double angle, double distance, XYPosition previousPosition);
        internal event MoveHandler? OnMove;
        public delegate void MoveCompleteHandler(GameObject sender);
        public event MoveCompleteHandler? MoveComplete;
        public delegate void MoveStartHandler(GameObject sender);
        public event MoveStartHandler? MoveStart;
        protected int lastMoveTime = 0;
        public virtual void Move(double angle, double distance)
        {
            if (double.IsNaN(angle))
                angle = 0;
            if (double.IsNaN(distance))
                distance = 0;
            if (distance == 0)
                return;
            if ((Environment.TickCount - lastMoveTime) < 800.0 / _frameRate)
                return;
            lastMoveTime = Environment.TickCount;
            Operations.Add(
                () =>
                //lock (privateLock)
                {
                    if (!this._movable)
                        return;
                    MoveStart?.Invoke(this);
                    XYPosition previousPosition = new XYPosition(Math.Round(_position.x, 6), Math.Round(_position.y, 6));
                    positiontmp = previousPosition + new XYPosition(distance * Math.Cos(angle), distance * Math.Sin(angle));
                    if (positiontmp.x <= 1000 && positiontmp.y <= 1000 && positiontmp.x >= 0 && positiontmp.y >= 0)
                    { _position = positiontmp;
                        point = new Point(_position); 
                    }
                   //Debug(this, "Move from " + previousPosition.ToString() + " angle : " + angle + " distance : " + distance + " aim : " 
                     //   +_position.ToString());
                    OnMove?.Invoke(this, angle, distance, previousPosition);
                    Debug(this, "Now pos : " + this.point.ToString());
                    MoveComplete?.Invoke(this);
                });
        }
        //Move end



        //Width
        private int _width = 1;
        public int Width
        {
            get => _width;
        }
        //Width end

        //Height
        private int _height = 1;
        public int Height
        {
            get => _height;
        }
        //Height end

        public GameObject(GameObject? parent = null)
        {
            ID = currentMaxID;
            currentMaxID++;
            this.Parent = parent;
            //MovingTimer.Change(0, 0);
            new System.Threading.Thread(//开辟线程实现
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

        public GameObject(XYPosition position, GameObject? parent = null) : this(parent)
        {  
            this.Position = position;
            this.point = new Point(position);
        }
        public static void Debug(GameObject current,string str) {
            Console.WriteLine(current.GetType()+""+str);
        }


    }
}

# THUnity2D

This is a C# project aiming to build a dll running on .NET Core 3.1. This dll contains all classes of game objects and the main logic of this game. The declarations of all classes in this project are as follows: 

> 这是一个致力于构建一个运行于.NET Core 3.1平台上的dll的C#项目。这个dll包含所有的游戏对象类以及游戏的主要逻辑。这个项目中所有类的声明如下：

```c#
namespace THUnity2D
{
    public struct XYPosition
    {
        public int x;
        public int y;
        public static XYPosition operator+ (XYPosition p1, XYPosition p2);
        public override string ToString();
        public XYPosition(int x, int y);
    }

    public struct Vector
    {
        public double angle;
        public double length;
        public static XYPosition Vector2XY(Vector v);
    }

    public struct Vector2XY
    {
        public double x;
        public double y;
        public static double operator* (Vector2 v1, Vector2 v2);    //The dot product of two vectors.
        public Vector2(double x, double y);
    }

    public static class Tools
    {
        public static double CorrectAngle(double angle);    //Change the angle to the main value: 0~2pi
    }

    public abstract class GameObject    //The base of all game objects
    {
        
        //For C++/CLI to debug, because I do not know how to use the operator 'is' in C++/CLI. [Doge]
        public enum GameObjType {}
        public abstract GameObjType GetGameObjType();

        //constants

        public const int MinSpeed = 1;
        public const int MaxSpeed = int.MaxValue;
        public const long invalidID = long.MaxValue;
        public const long noneID = long.MinValue;

        //The public lock.

        public readonly object moveLock;

        //Public properties

        public long ID { get; }
        public XYPosition Position { get; }
        public double FacingDirection { get; }
        public bool IsRigid { get; }
        public int MoveSpeed { get; set; }
        public bool CanMove { get; set; }
        public bool IsMoving { get; set; }
        public int Radius { get; }

        ///////////////////////////

        //Public Manners

        public long Move(Vector displacement);      //parameter: displacement; return value: the length that the object actually walks.
        public GameObject(XYPosition initPos, int radius, bool isRigid, int moveSpeed); //Constructor

        //private fields and manners

        //protected readonly BlockingCollection<Action> Operations = new BlockingCollection<Action>();
        private static long currentMaxID;
        private XYPosition position;
        private double facingDirection;
        private int _moveSpeed;
        private bool canMove = false;
        private bool isMoving;
        private int radius;

        //The protected lock to lock fields.
        
        protected readonly object gameObjLock;
    }

    public enum ObjType {}

    public class Obj : GameObject   //The base of non-character game objects
    {
        //Public fields and properties

        public readonly ObjType objType;
        public Character? Parent { get; set; }

        //Public manners

        public override string ToString();
        public Obj(XYPosition initPos, int radius, bool isRigid, int moveSpeed, ObjType objType);   //Constructor

        ///////////////////////////

        //private fields

        private Character? parent;
    }

    public enum PropType {}

    public class Prop : Obj
    {
        public readonly PropType propType;
        public Prop(XYPosition initPos, int radius, PropType propType);
    }

    public enum BulletType {}

    public sealed class Bullet : Obj
    {
        public Bullet(XYPosition initPos, int radius, int basicMoveSpeed, BulletType bulletType);
    }

    public class Wall : Obj
    {
        public Wall(XYPosition initPos, int radius);
    }

    public enum JobType {}

    public sealed class Character : GameObject
    {
        //Public fields and properties.

        public readonly JobType jobType;
        public readonly int orgAp;      //The initial power of attacking.
        public readonly BulletType bulletType;  //The type of buttets that the character has.
        
        public long TeamID { get; set; }
        public int CD { get; }
        public int MaxBulletNum { get; }
        public int BulletNum { get; }
        public int MaxHp { get; }
        public int HP { get; }
        public int AP { get; set; }
        public Prop? HoldProp { get; set; } //The prop that this player is holding.
        public int Score { get; }

        //Public manners

        public void ResetMoveSpeed();   //Set the speed to initial speed.
        public void AddBulletNum();     //Add the number of bullets for 1.
        public void AddHp(int add);
        public void SubHp(int sub);
        public bool Attack();           //return value: whether the attack is succeed or not. Effect: If the attack succeed, the number of its bullets will be substracted.
        public Prop? UseProp();         //return value: the prop that the player was holding. Effect: the prop that the player is holding will be cleared to null.
        public void AddScore(int add);
        public void SubScore(int sub);
        public Character(XYPosition initPos, int radius, JobType jobType, int basicMoveSpeed);  //Constructor.

        ///////////////////////////
        
        //Private fields

        private readonly int orgMoveSpeed;  //The initial speed.
        private long teamID;
        private int cd;
        private int maxBulletNum;
        private int bulletNum;
        private readonly int maxHp;
        private int hp;
        private int ap;
        private Prop? holdProp;
        private int score;

        //Private manners

        private bool TrySubBulletNum(); //Try substracting the number of bullets for 1. 
    }

    public class TeamID
    {
        public const long invalidTeamID;
        public const long noneTeamID;

        public static long CurrentMaxTeamID { get; }
        public long TeamID { get; }
        public int Score { get; }

        public void AddPlayer(Character player);
        public void OutPlayer(long playerID);
        public void ClearPlayer();
        public static bool teamExists(long findTeamID);     //Return value: if the team of ID "findTeamID" exists, it will return true; otherwise, it will return false.
        public Team();      //Constructor

        private static long currentMaxTeamID;
        private readonly long teamID;
        private ArrayList playerList;
    }

    public enum MapInfoObjType : uint {}

    public static class MapInfo
    {
        public static uint[,] map;  //Storage the initial information of the map of the game.
    }

    public class Map
    {

        //Some public enumerations and structures.

        public enum ColorType {}

        public struct PlayerInitInfo    //The structure to storage the infomation used to init a player.
        {
            public XYPosition initPos;
            public JobType jobType;
            public long teamID;

            public PlayerInitInfo(XYPosition initPos, JobType jobType, long teamID);    //Constructor.
        }

        //Public constants
        
        public const int numOfGridPerCell = 1000;       //The number of units of length per cell.
		public const int numOfStepPerSecond = 50;		//The number of steps that an ordinary player can walk for per second.

        //Public propperties

        public int Rows { get; }
        public int Cols { get; }
        public int ObjRadius { get; }
        public bool IsGaming { get; }

        //Public manners
        
        public XYPosition CellToGrid(int x, int y);     //Change the cell number to the corresponding coordinate.
        public int GridToCellX(XYPosition pos);         //Change the coordinate to the cell numeber of x.
        public int GridToCellX(XYPosition pos);         //Change the coordinate to the cell numeber of y.
        public long AddPlayer(PlayerInitInfo playerInitInfo);   //Add a player into the game according to "playerInitInfo". return value: If the addition succeeds, it will return the ID of the new player; otherwise, it will return GameObject.InvalidID.
        public bool StartGame(int milliSeconds);        //Start the game if the game isn't running, and it will BLOCK THE THREAD until the game ends if it seccessfully start this game. Parameter: the time that the game continues. Return value: If the game wasn't running and it secessfully started a game, it will return true; otherwise, it will return false.
        public void MovePlayer(long playerID, int moveTime, double moveDirection);  //Try to make the player with ID "playerID" walk towards "movedirection" for "moveTime".
        public void Attack(long playerID, int time, double angle);      //Try to make the player with the ID "playerID" to attack.
        public ArrayList GetGameObject();               //Return all game objects in the game.
        public Map(uint[,] mapResource, int numOfTeam); //Constructor.
        
        ///////////////////////////
        
        //Private fields

		private readonly int numOfTeam;
		private readonly int basicPlayerMoveSpeed;
        private bool isGaming;
        private ColorType[,] cellColor;     //Storage the color of every cells of the map.
        private ArrayList objList;          //The list of all game objects except the players.
        private object objListLock;
		private ArrayList playerList;       //The list of all players.
        private object playerListLock;
		private ArrayList teamList;         //The list of all teams.
        private object teamListLock;
        private void MoveObj(GameObject obj, int moveTime, double moveDirection);           //Try to move "obj" towards "moveDirection" for "moveTime".
        private GameObject? CheckCollision(GameObject obj, Vector moveVec);                 //Check whether obj will collide with other objects after it moves for the vector "moveVec".
        private bool OnCollision(GameObject obj, GameObject collisionObj, Vector moveVec);  //If the obj will collide with "collisionObj" after moving for "moveVec", this function will be called to deal with the collision.
        private void BulletBomb(Bullet bullet, GameObject? objBeingShot);   //If a bullet "bullet" will bomb as it collide with the player "objBeingShot" (none if no players), this function will be called.
    }

}

```


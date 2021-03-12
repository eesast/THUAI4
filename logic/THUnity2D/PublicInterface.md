# THUnity2D中各个类的接口

```c#
namespace THUnity2D
{
	public struct XYPosition
	{
		public int x;
		public int y;
		public static XYPosition operator +(XYPosition p1, XYPosition p2);
		public override string ToString();
		public XYPosition(int x, int y);							//Constructor
	}

	public struct Vector											//2-dim vector in polar coordinates
	{
		public double angle;
		public double length;
		public static XYPosition Vector2XY(Vector v);				//Transform Vector to XYPosition
		public static Vector XY2Vector(double x, double y);			//Transform (x, y) to Vector
		public Vector(double angle, double length);					//Constructor
	}

	public struct Vector2											//2-dim vector in Cartesian coordinates
	{
		public double x;
		public double y;
		public static double operator *(Vector2 v1, Vector2 v2);	//The dot product of two vectors.
		public static Vector2 operator +(Vector2 v1, Vector2 v2);	//The sum of two vectors
		public Vector2(double x, double y);							//Constructor
	}

	public static class Tools
	{
		public static double CorrectAngle(double angle);			//Change the angle to the main value: 0~2pi
	}

	public abstract class GameObject					//The base of all game objects
	{

		//For C++/CLI to debug, because I do not know how to use the operator 'is' in C++/CLI. [Doge]
		public enum GameObjType { }
		public enum ShapeType { }
		public abstract GameObjType GetGameObjType();

		public const int MinSpeed = 1;					//Minimal speed
		public const int MaxSpeed = int.MaxValue;		//Maximal speed

		protected readonly object gameObjLock;			//The lock for the object to change properties.
		public readonly object moveLock;				//The lock for the object to move.

		public const long invalidID = long.MaxValue;	//To identify that the ID is invalid.
		public const long noneID = long.MinValue;		//To identify that the ID doesn't exist.

		public long ID { get; }							//The ID of this object

		public XYPosition Position { get; protected set; }
		public readonly XYPosition orgPos;				//The position of this object when it is constructing.

		public double facingDirection { get; }

		public bool IsRigid { get; protected set; }		//Whether other objects can collide with it.

		protected ShapeType shape;
		public ShapeType Shape { get; }

		public int MoveSpeed { get; protected set; }
		public int OrgMoveSpeed { get; protected set; }	//The speed when it is constructing.

		public bool CanMove { get; set; }				//Whether the object can move.

		public bool IsMoving { get; set; }				//Whether the object is moving,

		public bool IsReseting { get; set; }			//Whether the object is resseting after dying.

		public bool IsAvailable { get; }				//Whether the object is available to accept orders, which is equal to it can move and it isn't moving or resetting.

		public long Move(Vector displacement);			//Change position, and return the square of the length moved for actually.

		public int Radius { get; }						//The radius of the circle or the inscribed circle of the square.

		public virtual void Reset();					//Reset this object after dying.

		public GameObject(XYPosition initPos, int radius, bool isRigid, int moveSpeed, ShapeType shape);	//Constructor
		public override string ToString();
		public static void Debug(GameObject current, string str);	//Output debug informations.

	}

	public enum ObjType { }

	public abstract class Obj : GameObject				//The base of non-character game objects
	{
		public override GameObjType GetGameObjType();
		public readonly ObjType objType;

		public Character? Parent { get; set; }			//The owner of the object.

		public Obj(XYPosition initPos, int radius, bool isRigid, int moveSpeed, ObjType objType, ShapeType shape);	//Constructor
		public override string ToString();

	}

	public class OutOfBoundBlock : Obj
	{
		public OutOfBoundBlock(XYPosition initPos);		//Constructor
	}

	public enum PropType { }

	public class Prop : Obj
	{
		public const int MinPropTypeNum;				//The minimal value of the members of the enumeration PropType.
		public const int MaxPropTypeNum;				//The maximal value of the members of the enumeration PropType.

		protected bool laid = false;					//Whether the prop has been laid.
		public bool Laid { get; }

		public abstract PropType GetPropType();

		public Prop(XYPosition initPos, int radius);	//Constructor

		public void ResetPosition(XYPosition pos);
		public void ResetMoveSpeed(int newMoveSpeed);
	}

	public enum BulletType { }

	public sealed class Bullet : Obj
	{
		public int AP { get; }

		public BulletType BulletType { get; }

		public Bullet(XYPosition initPos, int radius, int basicMoveSpeed, BulletType bulletType, int ap);

		public bool InColorRange(int CellX, int CellY);	//Judge if the point (int CellX, int CellY, int numOfGridPerCell) is in the range to be colored.

	}

	public abstract class Buff : Prop
	{
		public Buff(XYPosition initPos, int radius);	//Constructor
	}

	public abstract class Mine : Prop
	{
		public void SetLaid(XYPosition pos);			//Lay the mine.
		public Mine(XYPosition initPos, int radius);	//Constructor
	}

	public sealed class Buffs : Buff					//"Buffs" can be replaced with bike, Amplifier, JinKeLa, Rice, Shield, Totem and Spear.
	{
		public Buffs(XYPosition initPos, int radius);	//Constructor

		public override PropType GetPropType();
	}

	public sealed class Mines : Mine					//"Buffs" can be replaced with bike, Amplifier, JinKeLa, Rice, Shield, Totem and Spear.
	{
		public Mines(XYPosition initPos, int radius);	//Constructor

		public override PropType GetPropType();
	}

	public class Wall : Obj
	{
		public Wall(XYPosition initPos, int radius);	//Constructor
	}

	class BirthPoint : Obj
	{
		public BirthPoint(XYPosition initPos, int radius);	//Constructor
	}

	public enum JobType { }

	public sealed class Character : GameObject
	{
		public const int basicAp = 1000;
		public const int basicHp = 5000;
		public const int basicCD = 2000;
		public const int basicBulletNum = 15;

		public override GameObjType GetGameObjType();

		public long TeamID { get; set; }

		public readonly JobType jobType;

		public readonly object propLock;		//The lock for this character to modify the prop it is holding.
		public bool IsModifyingProp { get; set; }

		public int CD { get; }
		public readonly int orgCD;

		public int MaxBulletNum { get; }
		public int BulletNum { get; }
		public void AddBulletNum();

		public int MaxHp { get; }
		public int HP { get; }
		public void AddHp(int add);
		public int LifeNum { get; }				//The number of dying, unused.

		public void BeAttack(int subHP, bool hasSpear, Character? attacker);

		public const int MinAP = 0;
		public const int MaxAP = int.MaxValue;
		public readonly int orgAp;
		public int AP { get; }					//The power of attacking.

		public Prop? HoldProp { get; set; }

		public readonly BulletType bulletType;	//The type of the bullet that this character can shoot.

		public bool Attack();					//Try to attack for one time. If succeed, return true; otherwise, return false.

		public Prop? UseProp();					//Return the prop that is holding and set holdingProp to null.

		public int Score { get; }
		public void AddScore(int add);
		public void SubScore(int sub);

		public override void Reset();			//Reset this character after dying.

		public string Message { get; set; }		//Stage the message that its teammates last sent to it, with default value "THUAI4".

		#region To realize the effect of buffs.

		public void AddMoveSpeed(int add, int buffTime);
		public void AddAP(int add, int buffTime);
		public void ChangeCD(double discount, int buffTime);
		public void AddShield(int shieldTime);
		public bool HasShield { get; }
		public void AddTotem(int totemTime);
		public bool HasTotem { get; }
		public void AddSpear(int spearTime);
		public bool HasSpear { get; }

		#endregion

		public Character(XYPosition initPos, int radius, JobType jobType, int basicMoveSpeed);	//Constructor

	}

	public class Team
	{
		public static long CurrentMaxTeamID { get; }
		public long TeamID { get; }
		public const long invalidTeamID = long.MaxValue;	//To identify the ID is invalid.
		public const long noneTeamID = long.MinValue;		//To identify that it has no ID.

		public int Score { get; }							//The summary of the scores of all players in the team.

		public Character? GetPlayer(long playerID);			//Return the handle to the player having ID "playerID". If none, return null.
		public void AddPlayer(Character player);			//Add player to the playerList.
		public void OutPlayer(long playerID);				//Kick out the player with ID "playerID"
		public void ClearPlayer();							//Clear all players in the playerList.
		public long[] GetPlayerIDs();						//Get the IDs of all players in this team.

		public static bool teamExists(long findTeamID);		//Whether the team with ID "findTeamID" exists.

		public Team();										//Constructor
	}

	public static class MapInfo
	{

		public enum MapInfoObjType : uint { }
		public const int numOfBirthPoint;	//The number of the members of the enumeration MapInfoObjType.
		public static uint BirthPointEnumToIdx(MapInfoObjType birthPointEnum);	//Transform the enumeration of birthpoint to the index.
		public static bool ValidBirthPointIdx(uint birthPointIdx);				//Whether a index of a birthpoint is valid.

		public static uint[,] map;			//Storage the initial information of the map of the game.
	}

	public class Map
	{
		public static class Constant
		{
			public const int numOfGridPerCell = 1000;
			public const int numOfStepPerSecond = 50;
			public const int addScoreWhenKillOnePlayer = 10;
			public const int producePropTimeInterval = 20 * 1000;

			public const int basicPlayerMoveSpeed = numOfGridPerCell * 4;
			public const int basicBulletMoveSpeed = numOfGridPerCell * 6;
			public const int thrownPropMoveSpeed = numOfGridPerCell * 8;

			public const int objMaxRadius = numOfGridPerCell / 2;
			public const int playerRadius = objMaxRadius;
			public const int wallRadius = objMaxRadius;
			public const int birthPointRadius = objMaxRadius;
			public const int bulletRadius = objMaxRadius / 2;
			public const int unpickedPropRadius = objMaxRadius;

			public const int buffPropTime = 30 * 1000;
			public const int shieldTime = buffPropTime;
			public const int totemTime = buffPropTime;
			public const int spearTime = buffPropTime;

			public const int bikeMoveSpeedBuff = numOfGridPerCell;
			public const int amplifierAtkBuff = Character.basicAp * 2;
			public const double jinKeLaCdDiscount = 0.25;
			public const int riceHpAdd = Character.basicHp / 2;

			public const int mineTime = 60 * 1000;
			public const int dirtMoveSpeedDebuff = numOfGridPerCell;
			public const int attenuatorAtkDebuff = amplifierAtkBuff;
			public const double dividerCdDiscount = 4.0;

			public static XYPosition CellToGrid(int x, int y);		//Calculate the central coordinate of the cell [x, y]
			public static int GridToCellX(XYPosition pos);			//Calculate the x index of the cell.
			public static int GridToCellY(XYPosition pos);			//Calculate the y index of the cell.
		}

		public enum ColorType { }
		public const int maxTeamNum = 4;
		public ColorType TeamToColor(long teamID);	//Get the color of the given team.
		public long ColorToTeam;					//Get the team that has the given color.

		public struct PlayerInitInfo				//The structure to init a player.
		{
			public uint birthPointIdx;
			public JonType jobType;
			public long teamID;
			public PlayerInitInfo(uint birthPointIdx, JobType jobType, long teamID);	//Constructor
		}

		public ColorType[,] CellColor { get; }		//Get the color of each cell.
		public int Rows { get; }
		public int Cols { get; }

		public long AddPlayer(PlayerInitInfo playerInitInfo);	//Add a player to the game and return the new player's ID. If it fails, return GameObject.invalidID.

		public bool IsGaming { get; }				//Whether the game is running.

		public bool StartGame(int milliSeconds);	//Start the game. The time that the game will last is specified by the parameter milliSeconds.

		public void MovePlayer(long playerID, int moveTimeInMilliseconds, double moveDirection);

		public bool Attack(long playerID, int timeInMilliseconds, double angle);			//Let the player with ID "playerID" shoot a bullet.
		public bool Pick(long playerID, PropType propType);									//Let the player with ID "playerID" pick a prop of type "propType" that is in the same cell as the player.
		public void Use(long playerID);														//Let the player with ID "playerID" use the prop it is holding.

		public void SendMessage(long fromID, long toID, string message);					//Let the player with ID "fromID" send a message "message" to his teammate with ID "toID", and the length of the message should be no more than 64.

		public ArrayList GetGameObject();													//Get all object in this game.

		public Character GetPlayerFromTeam(long playerID);									//Get the player with ID "playerID" from "teamList". This action guarantees to find the player, otherwise it will throw an exception.

		public long[] GetPlayerIDsOfTheTeam(long teamID);									//Get all IDs of the players in the team with ID "teamID".

		public void Throw(long playerID, int moveTimeInMilliseconds, double angle);			//Let the player with ID "playerID" throw the prop it is holding.

		public int GetTeamScore(long teamID);												//Get the score of the team with ID "teamID".

		public Map(uint[,] mapResource, int numOfTeam);										//Constructor.
	}

}

```

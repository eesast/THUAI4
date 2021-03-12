# CAPI

## 关于AI.play()调用时机的说明

对于每一条MessageToClient对应的游戏状态，ai.play()**至多调用一次**。如果play跑得快，一次调用结束后下一条消息还没有来，ai线程会**阻塞**直到游戏状态更新。如果play跑得慢，下一次调用play时对应**最新的**游戏状态，**中间状态的信息不保存**。

## CAPI命令行参数

```bash
USAGE:

   CAPI  [-dhw] [--version] [-I <string>] [-f <string>] -P <USHORT> -j <0|1|
         2|3|4|5|6> -p <0|1|2|3> -t <0|1>


Where:

   -I <string>,  --agentIP <string>
     Agent`s IP 127.0.0.1 in default

   -P <USHORT>,  --agentPort <USHORT>
     (required) Port the agent listens to

   -p <0|1|2|3>,  --playerID <0|1|2|3>
     (required) Player ID 0,1,2,3 valid only

   -t <0|1>,  --teamID <0|1>
     (required) Team ID, which can only be 0 or 1

   -j <0|1|2|3|4|5|6>,  --jobType <0|1|2|3|4|5|6>
     (required) Player`s job, represented by an integer from 0 to 6
     balabala

   -d,  --debug
     Set this flag to use API for debugging.
     If "-f" is not set, the log will be printed on the screen.
     Or you could specify a file to store it.

   -f <string>,  --filename <string>
     Specify a file to store the log.

   -w,  --warning
     Warn of some obviously invalid operations (only when "-d" is set).


   -h,  --help
     Displays usage information and exits.
```



## API

```c++
class API final :public LogicInterface
{
private:
	virtual void StartTimer() {}
	virtual void EndTimer() {}
public:
	API(std::function<void(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp,
		const State*& pS);
	virtual void MovePlayer(uint32_t timeInMilliseconds, double angle);
	virtual void MoveRight(uint32_t timeInMilliseconds);
	virtual void MoveUp(uint32_t timeInMilliseconds);
	virtual void MoveLeft(uint32_t timeInMilliseconds);
	virtual void MoveDown(uint32_t timeInMilliseconds);
	virtual void Use();
	virtual void Pick(THUAI4::PropType propType);
	virtual void Throw(uint32_t timeInMilliseconds, double angle);
	virtual void Attack(uint32_t timeInMilliseconds, double angle);
	virtual void Send(int toPlayerID, std::string message);

	//Information the player can get
	virtual bool MessageAvailable();
	virtual bool TryGetMessage(std::string&);

	virtual std::vector<const THUAI4::Character*> GetCharacters() const;
	virtual std::vector<const THUAI4::Wall*> GetWalls() const;
	virtual std::vector<const THUAI4::Prop*> GetProps() const;
	virtual std::vector<const THUAI4::Bullet*> GetBullets() const;
	virtual std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const;
	virtual const THUAI4::Character& GetSelfInfo() const;
	virtual THUAI4::ColorType GetSelfTeamColor() const;
	virtual uint32_t GetTeamScore() const;
	virtual const std::array<std::array<uint32_t, StateConstant::nPlayers>, StateConstant::nTeams>& GetPlayerGUIDs() const override;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};
```



## Constants&Structures

```C++
namespace THUAI4
{

	enum class PropType : unsigned char
	{
		Null = 0,
		Bike = 1,
		Amplifier = 2,
		JinKeLa = 3,
		Rice = 4,
		NegativeFeedback = 5,
		Totem = 6,
		Phaser = 7,
		Dirt = 8,
		Attenuator = 9,
		Divider = 10
	};

	enum class ShapeType : unsigned char
	{
		Circle = 0,
		Square = 1
	};

	enum class JobType : unsigned char
	{
		Job0 = 0,
		Job1 = 1,
		Job2 = 2,
		Job3 = 3,
		Job4 = 4,
		Job5 = 5,
		Job6 = 6,
	};

	enum class BulletType : unsigned char
	{
		Bullet0 = 0,
		Bullet1 = 1,
		Bullet2 = 2,
		Bullet3 = 3,
		Bullet4 = 4,
		Bullet5 = 5,
		Bullet6 = 6
	};

	enum class ColorType : unsigned char
	{
		None = 0,
		Color1 = 1,
		Color2 = 2,
		Color3 = 3,
		Color4 = 4,
		Invisible = 5
	};

	struct Character
	{
		bool isMoving;
		bool isDying;
		ShapeType shapeType : 2;
		BulletType bulletType : 4;
		PropType propType : 4;
		JobType jobType : 4;
		uint16_t lifeNum;
		uint16_t teamID;
		uint16_t radius;
		uint16_t maxBulletNum;
		uint16_t bulletNum;

		uint32_t x;
		uint32_t y;
		uint32_t moveSpeed;
		uint32_t ap;
		uint32_t maxHp;
		uint32_t hp;
		uint32_t CD;
		uint64_t guid;
		double facingDirection;
	};

	struct Wall
	{
		ShapeType shapeType : 2;
		uint16_t radius;
		uint32_t x;
		uint32_t y;
		uint64_t guid;
	};

	struct Prop
	{
		bool isMoving;
		bool isLaid;
		ShapeType shapeType : 2;
		PropType propType : 4;
		uint16_t radius;
		uint32_t x;
		uint32_t y;
		uint32_t moveSpeed;
		uint64_t guid;
		double facingDirection;
	};

	struct Bullet
	{
		bool isMoving;
		ShapeType shapeType : 2;
		BulletType bulletType : 4;
		uint16_t radius;
		uint16_t teamID;
		uint32_t x;
		uint32_t y;
		uint32_t moveSpeed;
		uint32_t ap;
		uint64_t guid;
		double facingDirection;
	};

	struct BirthPoint
	{
		ShapeType shapeType;
		uint16_t teamID;
		uint16_t radius;
		uint32_t x;
		uint32_t y;
		uint64_t guid;
	};

}
```

```C++
#define M_SCI static const constexpr inline
#define MF_SCI static constexpr inline

namespace Constants
{
	using XYPosition = ::std::pair<std::int32_t, std::int32_t>;

	struct Map
	{

		M_SCI std::uint64_t sightRadius = 5000;
		M_SCI std::uint64_t sightRadiusSquared = sightRadius * sightRadius;
		M_SCI std::int32_t numOfGridPerCell = 1000;

		[[nodiscard]] MF_SCI auto CellToGrid(int x, int y) noexcept
		{
			return std::make_pair<std::int32_t, std::int32_t>(x * numOfGridPerCell + numOfGridPerCell / 2, y * numOfGridPerCell + numOfGridPerCell / 2);
		}

		[[nodiscard]] MF_SCI std::int32_t GridToCellX(XYPosition pos) noexcept
		{
			return pos.first / numOfGridPerCell;
		}

		[[nodiscard]] MF_SCI std::int32_t GridToCellY(XYPosition pos) noexcept
		{
			return pos.second / numOfGridPerCell;
		}
	};

	struct BulletMoveSpeed
	{
	private:

		M_SCI std::int32_t basicBulletMoveSpeed = Map::numOfGridPerCell * 6;

	public:

		M_SCI std::int32_t bullet0 = basicBulletMoveSpeed;
		M_SCI std::int32_t bullet1 = basicBulletMoveSpeed * 2;
		M_SCI std::int32_t bullet2 = basicBulletMoveSpeed / 2;
		M_SCI std::int32_t bullet3 = basicBulletMoveSpeed / 2;
		M_SCI std::int32_t bullet4 = basicBulletMoveSpeed * 4;
		M_SCI std::int32_t bullet5 = basicBulletMoveSpeed;
		M_SCI std::int32_t bullet6 = basicBulletMoveSpeed;
	};

	struct PropTimeInSeconds
	{

	private:

		M_SCI std::int32_t propTimeInSeconds = 30;
		M_SCI std::int32_t mineTimeInSeconds = 60;

	public:

		M_SCI std::int32_t bike = propTimeInSeconds;
		M_SCI std::int32_t amplifier = propTimeInSeconds;
		M_SCI std::int32_t jinKeLa = propTimeInSeconds;
		M_SCI std::int32_t rice = propTimeInSeconds;
		M_SCI std::int32_t shield = propTimeInSeconds;
		M_SCI std::int32_t totem = propTimeInSeconds;
		M_SCI std::int32_t spear = propTimeInSeconds;

		M_SCI std::int32_t dirt = mineTimeInSeconds;
		M_SCI std::int32_t attenuator = mineTimeInSeconds;
		M_SCI std::int32_t divider = mineTimeInSeconds;
	};

}

#undef MF_SCI
#undef M_SCI
```


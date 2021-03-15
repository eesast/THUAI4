# CAPI简介



## 关于 AI.play() 调用时机的说明



对于每一条 MessageToClient 对应的游戏状态，```AI.play() ```**至多调用一次**。如果 ```play()``` 跑得快，一次调用结束后下一条消息还没有来，AI 线程会**阻塞**直到游戏状态更新。如果 ```play()``` 跑得慢，下一次调用 ```play()``` 时对应**最新的**游戏状态，**中间状态的信息不保存**。



## CAPI命令行参数



Usage:

```bash
CAPI  [-dhw] [--version] [-I <string>] [-f <string>] -P <USHORT> \
-j <0|1|2|3|4|5|6> -p <0|1|2|3> -t <0|1>
```

- ```-I/--agentIP```: Agent的 IP 地址，默认值是127.0.0.1
- ```-P/--agentPort```: Agent监听的端口号。
- ```-p/--playerID```: 玩家 ID ，只能是0至3的整数。同一队伍中两人的ID不能相同。
- ```-t/--teamID```: 队伍 ID ，只能是0或1。两队ID不能相同。
- ```-j/--jobType```: 玩家职业，只能是0到6的整数，对应玩家不同职业。
- ```-d/--debug```: 使用debug版本的 API（其实就是生成 log ）。Log 记录每次```AI.play()```调用所用时间和期间各 API 函数的调用情况。
- ```-f/--filename```: 默认 log 输出到屏幕上，也可以通过该参数指定一个文件夹作为 log 的储存位置。该参数仅当```-d```被指定时有效。
- ```-w/--warning```: 调用 API 函数时会检查一些比较明显的错误。仅当```-d```被指定时有效。
- ```-h/--help```:  输出对命令行参数的解释。

其中，```-p```、```-t```、```-j```、```-P```必须指定。

举个例子：

```bash
./CAPI.exe -I 127.0.0.1 -P 7777 -p 0 -t 0 -j 0
```

即连接在本机7777端口监听的 Agent ，玩家 ID 与队伍 ID 都是0，游戏中的职业为职业0。



## API介绍



### 选手主动行为

除发消息以外，所有主动行为只有角色存活时才能进行。



#### 移动

```C++
void MovePlayer(uint32_t timeInMilliseconds, double angle);
```

进行玩家移动操作。第一个参数是移动时间，以毫秒为单位。第二个参数是方位角（弧度制，虽然angle的英文解释是：the space between two lines or surfaces that join, measured in degrees ）。单位圆上，方位角$\theta$与$xy$坐标的关系是：$x=\cos\theta,y=\sin\theta$，其中$x$轴以向下为正，$y$轴以向右为正。后文不做特殊说明，方位角的含义均同此处。

---



```c++
void MoveRight(uint32_t timeInMilliseconds);
```

向右移动，即向$y$轴正方向移动。参数是移动时间，以毫秒为单位。下同。

---



```c++
void MoveUp(uint32_t timeInMilliseconds);
```

向上移动，即向$x$轴负方向移动。

---




```c++
void MoveLeft(uint32_t timeInMilliseconds);
```

向左移动，即向$y$轴负方向移动。

---




```c++
void MoveDown(uint32_t timeInMilliseconds);
```

向下移动，即向$x$轴正方向移动。



#### 道具操作

```c++
void Use();
```

使用道具。仅当人物手中有道具时有效。

---



```c++
void Pick(THUAI4::PropType propType);
```

捡起道具，参数为道具类型，只能捡起与**人物中心**在**同一大格**内的道具。仅当格子内有对应种类的道具时有效。当多人同时拾取时，逻辑先处理谁发来的（合法）请求谁得到道具，所以```Pick()```是有可能失败的。

---



```c++
void Throw(uint32_t timeInMilliseconds, double angle);
```

扔道具。第一个参数是道具飞行的时间，以毫秒为单位。第二个参数是投掷的方位角。仅当人物手中有道具时有效。



#### 攻击

```c++
void Attack(uint32_t timeInMilliseconds, double angle);
```

射出子弹。第一个参数是投掷时间（毫秒计），只对某些职业有效。第二个参数是射击的方位角。仅当弹夹不为空时有效。



#### 发送消息

```c++
void Send(int toPlayerID, std::string message);
```

给队友发送消息。第一个参数是队友ID，第二个参数是要发送的消息。



### 选手可获得信息



**注意！！！**接口中返回的指针或引用指向的内存只保证在**本次```AI.play()```调用期间**是不变的。请不要试图保存这些地址并在其他时间访问。



#### 个人信息

```c++
const THUAI4::Character& GetSelfInfo() const;
```

返回一个```Character```结构体的常引用，其中包含了选手对应角色的各种信息。



#### 本队信息



```c++
THUAI4::ColorType GetSelfTeamColor() const;
```
返回本队颜色。

---



```c++
uint32_t GetTeamScore() const;
```

返回本队当前分数。



#### 玩家GUID信息



```c++
const std::array<std::array<uint64_t, 4>, 2>& GetPlayerGUIDs() const;
```

返回一个$2\times4$的array的引用（当成二维数组就好），两个维度分别对应队伍 ID 和玩家 ID ，元素是玩家的GUID。如果游戏玩家数少于$2\times4$，会存在对应玩家不存在的情况。此时GUID为Constants中定义的invalidGUID。



#### 可见物体信息



```c++
std::vector<const THUAI4::Character*> GetCharacters() const;
```
获得**视野范围内**所有玩家的信息。

返回值：元素为指向```Character```的常指针的vector。可能为空，顺序无意义。

---



```c++
std::vector<const THUAI4::Wall*> GetWalls() const;
```
获得**视野范围内**所有墙体的信息。

返回值：元素为指向```Wall```的常指针的vector。可能为空，顺序无意义。

---



```c++
std::vector<const THUAI4::Prop*> GetProps() const;
```
获得**视野范围内**所有**可见**道具的信息。

返回值：元素为指向```Prop```的常指针的vector。可能为空，顺序无意义。

---



```c++
std::vector<const THUAI4::Bullet*> GetBullets() const;
```

获得**视野范围内**所有子弹的信息。

返回值：元素为指向```Bullet```的常指针的vector。可能为空，顺序无意义。

---



```c++
std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const;
```
获得**视野范围内**所有出生点的信息。

返回值：元素为指向```BirthPoint```的常指针的vector。可能为空，顺序无意义。



#### 地图颜色信息



```c++
THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
```

获得$(CellX,CellY)$格的颜色，返回一个```THUAI4::ColorType```枚举型。、。如果在视野范围之外，会返回```THUAI4::ColorType::Invisible```。





#### 队友发来消息



注：底层实现是队列，所以收到多条消息不会覆盖而是按接收顺序保存。



```c++
bool MessageAvailable();
```

返回是否有未处理的消息。即消息队列为空时返回0，否则返回1。



```c++
bool TryGetMessage(std::string& buffer);
```

试图从消息队列队首pop出一条消息。如果成功返回1，队首消息覆盖buffer。失败返回0，buffer内容未定义。




## 数据结构

```C++
namespace THUAI4
{

	enum class PropType : unsigned char    //道具类型
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

	enum class ShapeType : unsigned char    //物体形状
	{
		Circle = 0,
		Square = 1
	};

	enum class JobType : unsigned char    //玩家职业
	{
		Job0 = 0,
		Job1 = 1,
		Job2 = 2,
		Job3 = 3,
		Job4 = 4,
		Job5 = 5,
		Job6 = 6,
	};

	enum class BulletType : unsigned char    //子弹种类
	{
		Bullet0 = 0,
		Bullet1 = 1,
		Bullet2 = 2,
		Bullet3 = 3,
		Bullet4 = 4,
		Bullet5 = 5,
		Bullet6 = 6
	};

	enum class ColorType : unsigned char    //颜色种类
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
		bool isMoving;  //是否正在移动
		bool isDying;  //是否正在复活中
		ShapeType shapeType : 2;  //形状（圆）
		BulletType bulletType : 4;  //子弹种类
		PropType propType : 4;  //所拥有道具类型
		JobType jobType : 4;  //职业类型
		uint16_t lifeNum;  //第几条命
		uint16_t teamID;  //队伍ID
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint16_t maxBulletNum;  //弹夹最大容量
		uint16_t bulletNum;  //当前子弹数量

		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		uint32_t moveSpeed;  //移动速度
		uint32_t ap;  //攻击力
		uint32_t maxHp;  //最大生命值
		uint32_t hp;  //当前声明值
		uint32_t CD;  //设计冷却时间
		int64_t guid;  //Global Unique Identifier
		double facingDirection;  //移动朝向
	};

	struct Wall
	{
		ShapeType shapeType : 2;  //形状（正方形）
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		int64_t guid;  //Global Unique Identifier
	};

	struct Prop
	{
		bool isMoving;  //是否正在移动
		bool isLaid;  //是否已被放置（选手能看到的一定没被放置）
		ShapeType shapeType : 2;  //形状
		PropType propType : 4;  //道具种类
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		uint32_t moveSpeed;  //移动速度
		int64_t guid;  //Global Unique Identifier
		double facingDirection;  //移动朝向
	};

	struct Bullet
	{
		bool isMoving;  //是否在移动（是）
		ShapeType shapeType : 2;  //形状（圆）
		BulletType bulletType : 4;  //子弹种类
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint16_t teamID;  //所属队伍
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		uint32_t moveSpeed;  //移动速度
		uint32_t ap;  //攻击力
		int64_t guid;  //Global Unique Identifier
		double facingDirection;  //移动朝向
	};

	struct BirthPoint
	{
		ShapeType shapeType;  //形状（圆）
		uint16_t teamID;  //队伍ID
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		int64_t guid;  //Global Unique Identifier
	};

}
```

## 常数

```C++
#define M_SCI static const constexpr inline
#define MF_SCI static constexpr inline

namespace Constants
{
	using XYPosition = ::std::pair<std::int32_t, std::int32_t>;

	struct Map
	{

		M_SCI std::uint64_t sightRadius = 5000;  //视野半径
		M_SCI std::uint64_t sightRadiusSquared = sightRadius * sightRadius;//视野半径的平方
		M_SCI std::int32_t numOfGridPerCell = 1000;  //每一大格有多少小格
		 //大格中心用小格表示的辅助函数
		[[nodiscard]] MF_SCI auto CellToGrid(int x, int y) noexcept 
		{
			return std::make_pair<std::int32_t, std::int32_t>(x * numOfGridPerCell + numOfGridPerCell / 2, y * numOfGridPerCell + numOfGridPerCell / 2);
		}
		//小格横坐标转为大格坐标的辅助函数
		[[nodiscard]] MF_SCI std::int32_t GridToCellX(XYPosition pos) noexcept
		{
			return pos.first / numOfGridPerCell;
		}
		//小格纵坐标转为大格坐标的辅助函数
		[[nodiscard]] MF_SCI std::int32_t GridToCellY(XYPosition pos) noexcept
		{
			return pos.second / numOfGridPerCell;
		}
	};

	struct BulletMoveSpeed  //子弹移速
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

	struct PropTimeInSeconds  //道具buff持续时间，以秒计（然而需要选手自己计时hhh）
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
	M_SCI int64_t invalidGUID = (std::numeric_limits<int64_t>::max)();  //invalidGuid
}

#undef MF_SCI
#undef M_SCI
```


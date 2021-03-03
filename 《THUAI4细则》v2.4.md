# 《THUAI4》细则 v2.4

---

**本次修改：关于队友通信的 proto**

---

## 一、通信接口建议

---

设想通信过程：  

由 Server 设置队伍数量 `teamCount` 和每队人数 `playerCount`，以及游戏持续时间 `time`，这样可以只用一个 Agent 来专门收发消息。游戏开始前，每个 Client 询问玩家要加入的队伍（用一个整数 `teamID` 表示，其中 `0 <= teamID < teamCount`），以及队伍的玩家号 `playerID`，其中 `0 <= playerID < playerCount`，以及玩家所选的职业（也用整数表示，详情参见下面的说明），玩家选择完毕后，向 `Server` 发送 `AddPlayer` 消息，然后进入等待。如果队伍、职业或玩家号无效（即玩家已存在或指定的 ID 超出范围），将会接收到 Server 发来的 `InvalidPlayer` 消息，这时 `Client` 可以重新询问玩家，或者抛异常结束，等等；如果有效，Client 将会接收到 Server 发来的 `ValidPlayer` 消息，然后等待 `Server` 发来开始游戏 `StartGame` 的消息后就开始游戏。  

当 `Server` 处组满队之后，就向所有的 `Client` 发送 `StartGame` 消息，`Client`端即可开始执行选手代码或，者开始玩家的鼠标键盘操作进行游戏。  

当时间到达后，`Server` 就向所有的 `Client` 发送 `EndGame` 消息，`Client` 随即停止游戏。  

其中，还有 `Send 消息，用于队友通信。

根据这个思路，编写如下通信接口：

### 实现的proto

```protobuf
enum MessageType
{
    AddPlayer = 0;
    Move = 1;
    Attack = 2;
    Pick = 3;
    Use = 4;
    Throw = 5;
    Send = 6;
    InvalidPlayer = 7;
    ValidPlayer = 8;
    StartGame = 9;
    Gaming = 10;
    EndGame = 11;
}

enum GameObjType
{
    Character = 0;
    Wall = 1;
    Prop = 2;
    Bullet = 3;
    BirthPoint = 4;
    OutOfBoundBlock = 5;
}

//道具类型，此为编写时的临时名称，具体道具名称可以待主题确定后更改
enum PropType
{
    Null = 0;
    Bike = 1;
    Amplifier = 2;
    JinKeLa = 3;
    Rice = 4;
    NegativeFeedback = 5;
    Totem = 6;
    Phaser = 7;
    Dirt = 8;
    Attenuator = 9;
    Divider = 10;
}

enum ShapeType
{
	Circle = 0;
	Square = 1;
}



message GameObjInfo
{
    GameObjType gameObjType = 1;
    
    //以下当gameObjType为任意值时均时有效
    int64 guid = 2;			//每个游戏对象有一个全局ID，即guid
    int32 x = 3;
    int32 y = 4;
    double facingDirection = 5;
    int32 moveSpeed = 6;
    bool canMove = 7;
    bool isMoving = 8;
    ShapeType shapeType = 9;	//形状
    int32 radius = 10;		//圆形物体的半径或正方形内切圆半径
    int64 teamID = 11;		//人物所属队伍ID或子弹、道具的发射主人所属队伍ID或出生点
    
    //以下当gameObjType为Character和Bullet时才有效
    
    int32 ap = 12;			//攻击力
    BulletType bulletType = 13;	//子弹类型
    
    //以下当gameObjType为Character和Prop时有效
    PropType propType = 14;	//当前人物持有的道具类型或道具的类型
    
    //以下仅当gameObjType为Prop时有效
    bool isLaid = 15;		//是否已经被放置
    
    //以下仅当gameObjType为Character时有效
    bool isDying = 16;		//正在死亡复活中……
    JobType jobType = 17;
    int32 CD = 18;
    int32 maxBulletNum = 19;
    int32 bulletNum = 20;
    int32 maxHp = 21;
    int32 hp = 22;
    int32 lifeNum = 23;		//第几次复活
}

//发送单个信息时
message MessageToOneClient
{
    int64 playerID = 1;			//指明发送给谁
    int64 teamID = 2;			//指明发送给谁
    
    MessageType messageType = 3;
    int64 guid = 4;             //自己的guid
    string message = 5;         //如果 messageType 为 Send，则为要发送的消息
}

//发送全局信息时
message MessageToClient
{
    int64 playerID = 1;			//指明发送给谁
    int64 teamID = 2;			//指明发送给谁
    
    MessageType messageType = 3;
    GameObjInfo selfInfo = 4;	//自己的个人信息
    ColorType selfTeamColor = 6;	//自己队伍的所属颜色
    repeated GameObjInfo gameObjs = 7;		//当前地图上的所有对象

    message OneTeamGUIDs
    {
        repeated int64 teammateGUIDs = 1;
    }

    repeated OneTeamGUIDs PlayerGUIDs = 5;  //所有玩家的GUID，第一维的 0、1、2…… 分别为队伍编号，每个队伍的0、1、2、……元素分别为playerID为0、1、2、……的玩家的GUID，若不存在该玩家，则为Constant::Constant::InvalidGUID

    message OneDimVec
    {
        repeated ColorType rowColors = 1;
    }

    repeated OneDimVec cellColors = 8;	//每个 cell 的颜色
    int32 teamScore = 9;                //本队伍的分数
}

message MessageToServer
{
    MessageType messageType = 1;
    JobType jobType = 2;	//messageType为AddPlayer时选择的职业
    int64 teamID = 3;		//messageType为AddPlayer时选择的队伍
    PropType propType = 4;	//messageType为Pick时要捡起的道具类型
    int32 timeInMilliseconds = 5;	//时间参数
    double angle = 6;		//角度参数
    int64 playerID = 7;     //发送的玩家ID
    string message = 8;     //如果messageType为Send，则为要发送的消息
    int64 SendToPlayerID = 9;     //如果messageType为Send，则为要发送对象的 playerID（本队伍）
}

```

从而通信组分别实现对 Server 和 Client 的发送消息方法，以及接收消息事件，两个接口，以及 Agent。

---

## 二、选手接口建议

建议给选手提供如下 C++ 接口（以及界面组实现相应的鼠标键盘操作接口）：  

1. 返回最近一次收到的来自 Server 的信息中的 `selfInfo`、`teammateIDs`、`selfTeamColor`，均为只读。  

2. `Constant` 类中的静态成员以及静态成员函数，具体数值与 logic 中的 `THUnity2D.Map.Constant` 类里的常成员保持一致，供选手只读访问。需要实现的变量和函数有：  

   ```cpp
   //
   // File: Constant.h
   //
   
   #include <cstdint>
   #include <utility>
   #include <limits>
   
   #define M_SCI static constexpr inline
   #define MF_SI static inline
   
   namespace Constant
   {
       using XYPosition = ::std::pair<int, int>;
   
       struct Constant
       {
           M_SCI int64_t invalidGUID = std::numeric_limits<int64_t>::max();
           M_SCI int32_t numOfGridPerCell = 1000;
           M_SCI int32_t buffPropTime = 30 * 1000;
           M_SCI int32_t shieldTime = 30 * 1000;
           M_SCI int32_t totemTime = 30 * 1000;
           M_SCI int32_t spearTime = 30 * 1000;
           M_SCI int32_t bikeMoveSpeedBuff = 1000;
           
           /*除此以外，还需实现的常量有amplifierAtkBuff、jinKeLaCdDiscount、riceHpAdd、mineTime、dirtMoveSpeedDebuff、attenuatorAtkDebuff、dividerCdDiscount，和上面的一样，具体数值参见 Logic 中 Map.cs 中的 THUnity2D.Map.Constant 类*/
   
           MF_SI auto CellToGrid(int x, int y) { return std::make_pair<int, int>(x * numOfGridPerCell + numOfGridPerCell / 2, y * numOfGridPerCell + numOfGridPerCell / 2); }
           MF_SI int32_t GridToCellX(XYPosition pos)
           {
               return pos.first / numOfGridPerCell;
           }
           MF_SI int32_t GridToCellY(XYPosition pos)
           {
               return pos.second / numOfGridPerCell;
           }
       };
   }
   
   #undef MF_SI
   #undef M_SCI
   
   ```

   

3. `std::list<GameObjInfo> GetGameObj(int cellX, int cellY)` 返回位于参数所示的 cell 中的所有可见的游戏对象的列表。（函数中实现视野，**并屏蔽掉已经被放置的地雷**）

4. `ColorType GetColor(int cellX, int cellY)` 返回 `cellColors[cellX][cellY]` 。

5. `void MovePlayer(int timeInMilliseconds, double angle)`、

   `void MoveRight(int timeInMilliseconds) { MovePlayer(timeInMilliseconds, PI * 0.5) }`、

   `void MoveUp(int timeInMilliseconds) { MovePlayer(timeInMilliseconds, PI) }`、

   `void MoveLeft(int timeInMilliseconds) { MovePlayer(timeInMilliseconds, PI * 1.5) }`、

   `void MoveDown(int timeInMilliseconds) { MovePlayer(timeInMilliseconds, 0.0) }`

6. `void Use()`、

   `void Pick(PropType propType)`、

   `void Throw(int timeInMilliseconds, double angle)`、

   `void Attack(int timeInMilliseconds, double angle)`、
   
   `void Send(int playerID, std::string message)`

---

## 三、游戏细则

---

 ### 游戏名称

+ **游戏名称**：待定

---

 ### 队伍组成

+ **队伍数量**：4（或2）

+ **队伍人数**：2（或4）

---

 ### 游戏简介

在游戏中，每个队员手中都有一杆枪，枪中装的子弹是五颜六色的墨水，每队的各个队员的墨水颜色都是一致的。这种墨水子弹在对落点处的非同色的队员造成一定伤害的同时，也会把周围的一片区域染成相应的颜色。如果一个队员被击中次数过多则会死亡。

---

 ### 地图

+ **地图大小与形状**：矩形场地。在地图上的游戏对象的位置由坐标 (x, y) 确定，其中 x 和 y 都是整数。其中每个坐标轴的每 1000 个坐标长度作为一个单位，两个坐标轴的每个单位之间交叉形成一个基本单元 (cell) ，即每个 cell 是一个 1000 x 1000 的正方形。地图由 50 x 50 = 2500 个 cell 组成。cell 的编号 (cellX, cellY) 可以由公式：$cellX=\frac{x}{numOfGridPerCell}$、$cellY=\frac{y}{numOfGridPerCell}$（其中 $numOfGridPerCell=1000$）计算得到。

  采用的坐标轴为竖直向下为 x 轴正向，水平向右为 y 轴正向。

---

 ### 人物

+ **移动方式**：  

  + 可以向任意方向移动，移动采用极坐标： `MovePlayer(time, rad)` 以x轴正方向为极轴正方向，逆时针为角度正方向，向`rad`弧度方向移动`time`时间（毫秒）  
  + 简易移动方式：`MoveUp(time)`、`MoveDown(time)`、`MoveLeft(time)`、`MoveRight(time)`可以向上下左右四个方向移动`time`时间（毫秒）  

+ **视野**：人物可以通过函数`GetMapObj(cellX, cellY)`搜寻地图上位于`(cellX, cellY)`格子内的所有可见物品（此处x和y为除以 $numOfGridPerCell$ 以后的值，即一大格）。人物有一定视野，超过视野的物体是不可见的。  

+ **形状**：每个人物是一个圆

+ **子弹**：当子弹爆炸时，会将周围一定范围内的颜色染成自己队伍所属的颜色。染色以 cell 为基本单位，即每个 cell 内颜色是相同的。此外，子弹爆炸还会导致一定范围内的玩家受到伤害，并可能会引起周围子弹的爆炸。每个人物的子弹数量是有限的。当人物所踩的 cell 的颜色是自己队伍的颜色时，会缓慢回复子弹数量。子弹是圆形物体，比人物稍小。

+ **属性**：ID、中心的坐标、面对方向、移动速度、碰撞半径、所属队伍ID、职业、回复子弹CD、

+ **队伍**：玩家可以通过函数 `GetTeammatesIDs()`函数获取所有队友的ID。

+ **职业**：（由选手在编写代码时设定，职业对于任何角色是**不可见**的）

  + 职业0：无职业，向某一方向发射，遇到障碍或到达地图边界时爆炸，各种属性均一般。攻击时仅 `angle` 参数有效。  

  1. 职业1：投掷型武器，子弹飞行的过程中不受任何干扰，直接命中目标点，伤害与染色范围一般，子弹飞行速度快，装弹CD一般，生命值一般，人物移动速度一般。两个参数均有效。  
  2. 职业2：导弹型武器，可以向某一方向发射，遇到障碍或到达地图边界或达到射程上限时爆炸，伤害大，伤害范围和染色范围大，子弹飞行速度慢，装弹CD长，生命值较高，人物移动速度较慢。两个参数均有效。  
  3. 职业3：拖泥带水型武器，子弹沿某一方向发射，会把子弹经过的路线全部染色，遇到障碍时或到达地图边界时爆炸，造成的伤害小，伤害范围小，爆炸处染色范围大，子弹飞行速度慢，装弹CD长，生命值较高，人物移动速度较快。仅 `angle` 参数有效。  
  4. 职业4：攻击型武器，子弹飞行不受任何干扰，直接命中目标点，子弹飞行速度极快，对敌人伤害极大，攻击范围极大，但染色范围小，装弹CD长，生命值较低，人物移动速度极快。两个参数均有效。  
  6. 职业5：近战型武器。子弹使用后立即爆炸，爆炸时会把人物面前的一定范围内染色，攻击范围一般，伤害极高，装弹CD长，生命值较低，人物移动速度极快。两个参数均无效。
  7. 职业6：反弹伤害。与无职业子弹类型相同，生命值较高，移动速度慢。反弹伤害为受到伤害的一半，但是反弹不会致死，最多会使遭到反弹者hp降为1点。仅 `angle` 参数有效。
  
+ **主动行为**：  

  + 移动。
  + 攻击。
  + 捡道具。
  + 使用道具。
  + 丢弃道具。

---

 ### 出生点

人物会在地图上的出生点出生，出生点是一个与人物等大的圆，只有属于该出生点的玩家才能进入该出生点，不属于该出生点的玩家会与出生点发生碰撞。出生点所属 cell  的颜色永远是该玩家所属队伍的颜色，永远不会被染色。

---

### 墙体

墙是一个正方形，可以起道阻碍作用。

---

 ### 道具

+ **简介**：道具会在地图的任意一个没有碰撞障碍、没有出生点的 cell 中随机生成   

+ **捡道具**：人物可以捡起与**人物的中心**处在同一 `cell` 内的道具

+ **具体道具**（以下的名字是编写时使用的临时名字，可以在确定主题后修改）：
  
  + buff类道具
    1. 自行车（Bike）: 增加一段时间的移动速度，效果可以叠加。
    2. 运算放大器（Amplifier）：增加一段时间的子弹对敌人的伤害（以子弹从手中发射的时刻计），效果可以叠加。  
    3. 金坷垃（JinKeLa）：一段时间内装弹CD减短，效果可以叠加。
    4. 烤肉拌饭（Rice）：回复大量的生命值
    5. 负反馈网络（NegativeFeedback）：使用后自己在一段时间内免受伤害。若连续使用盾牌会刷新盾牌持续时间。  
    6. 不死图腾（Totem）：使用后一段时间内消失，如果在消失之前如果被击中死亡，则原地满血复活，就像没死过一样，但生效后会消失。若连续使用不死图腾会刷新不死图腾持续时间。  
    7. 矛【移相器（Phaser）】：使用后一段时间内发射出的子弹无视盾的效果（时间以发射子弹的时刻计）
    
  + 地雷类道具
  
    + 地雷类道具综述：使用时会在自己脚下埋藏一颗地雷，该地雷在一定时间内存在。若在该段时间内有人踩在上面，则地雷对该角色起作用并消失；若无，则时间到后地雷自动消失。地雷被放置后是圆形物体
  
    1. 泥沼（Dirt）：效果与Bike相反
    2. 衰减器（Stagnancy/Attenuator）：与Amplifier相反
    3. 分频器（(Frequency) Divider）：效果与金坷垃相反
  
  

---

 ### 队友通信

`SendMessage(playerID, string)`可以向队友发一条字符串，发送后消息立即保存在队友的消息栏中，且会覆盖掉之前消息栏存在的消息。队友可以通过`string GetMMessage()`进行查看。每个人的消息栏存有的初始消息为`"THUAI4"` 。消息的长度不能超过 64 个字符，否则发送失败。 

---

 ### 死亡处理

角色死亡后将会先消失一秒钟，然后回到自己的出生点，并且失去所有的 buff 和 debuff 效果。消失的过程中人物不能进行任何行为。  

---

 ### 结算

得分结算方式：一个地图上的染着本队颜色的 cell 的数目成正比的量，加上一个与队伍内每个队员击杀数目成正比的量。

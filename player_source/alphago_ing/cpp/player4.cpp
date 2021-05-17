//happyman

#include"AI.h"
#include"Constants.h"
#include"Structures.h"

#include<iostream>
#include<vector>
#include<list>
#include<chrono>

#include<malloc.h>
#include<stdexcept>
#include<cstdio>
#include<cstdlib>
#include<random>

using namespace std;

//无符号整数
typedef unsigned char				u8;
typedef unsigned short int			u16;
typedef unsigned int				u32;
typedef unsigned long long int		u64;

//重要设置
#define MULTIPLE_PLAYER_MODE_									//用这个宏来选择游戏模式：多个玩家还是单个
extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1;	//选手职业，选手 !!必须!! 定义此变量来选择职业
extern const bool asynchronous = true;							//我的写法必须为true

//时间模块
u64 startTime;													//游戏起始时间
#define NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now ().time_since_epoch ()).count ()
#define EPOCHS_SINCE_START (((NOW) - startTime) / 50)			//以50ms为单位的时间
#define TIME_SINCE_START (NOW - startTime)						//以ms为单位的时间

//特殊指针
typedef vector<shared_ptr<const THUAI4::Wall>> WALLS_PTR;

//常数
constexpr inline double PI = 3.14159265358979323846;		//常数PI
constexpr inline double twoPI = 2 * PI;
#define SQRT_2						1.4142135623730951
#define radiusInCells				5
#define radiusInCellsSquared		25
#define sightRadius					5000

//随机数引擎
default_random_engine e;
uniform_real_distribution<double> rd(0, twoPI);

//重要的全局变量
THUAI4::Character self;											//自己的信息
THUAI4::ColorType teamColor = THUAI4::ColorType::None;			//本队的颜色
u32 teamScore = 1;
u32 PlayerNum = 4;												//本局游戏的玩家数量
int gameTimeInSeconds = 600;									//600s的比赛

//地图相关
static u8 Map[50][50];											//基本的地图数据
static double MapValue[50][50];									//地图评分
static int lastExplorationTime[50][50];							//上次探索时间
int lastAttackTimes[50][50];									//上次攻击时间，防止重复攻击
void InitMap();													//初始化地图

//查看地图状态使用的宏
#define isWall(x,y)				(Map[x][y] & 1)
#define isBirthPoint(x,y)		((Map[x][y] & 2)>>1)
#define isEmpty(x,y)			((!isWall(x,y)) && !isBirthPoint(x,y))
#define isExplored(x,y)			((Map[x][y] & 4) >> 2)
#define isColored(x,y)			(((Map[x][y] & 8) >> 3))
#define isMyColor(x,y)			(isColored(x, y) && ((Map[x][y] & 16) >> 4))
#define isSelfExplored(x,y)		((Map[x][y] & 64) >> 6)

#define isExploredInSimu(x,y)	((Map[x][y] & 16) >> 4)
#define Exists(x,y)				((Map[x][y] & 32) >> 5)

//改变地图状态
#define ExploreCell(x,y)		(Map[x][y] = (Map[x][y] | (1 << 2)))
#define SelfExploreCell(x,y)	(Map[x][y] = (Map[x][y] | (1 << 6)))
#define MarkWall(x,y)			(Map[x][y] = (Map[x][y] | 1))

#define ExploreInSimu(x,y)		(Map[x][y] = (Map[x][y] | (1 << 4)))
#define UnexploreInSimu(x,y)	(Map[x][y] = (Map[x][y] & ~(1 << 4)))
#define toExist(x,y)			(Map[x][y] = (Map[x][y] | (1 << 5)))
#define unExist(x,y)			(Map[x][y] = (Map[x][y] & ~(1 << 5)))


void toExplore(u32 coordX, u32 coordY);							//把给定坐标的视野内的cell改变为explored
void updateLocalMap(WALLS_PTR& visibleWalls);					//从获取的墙壁信息来标记出地图上的墙壁
void printMap();												//打印地图（测试）

#define CharaInfoLen 12											//用于同步的玩家数据	
struct SimplifiedCharacter {
	void PrintChara();
	SimplifiedCharacter(THUAI4::Character&);
	SimplifiedCharacter() = default;
	void upDate(THUAI4::Character&);
	// 数据：12个字节
	u8 jobType : 3;
	u8 propType : 4;
	u8 placeholder1 : 1;
	u8 isDying : 1;
	u8 bulletNum : 6;
	u8 placeholder2 : 1;
	u16 x;
	u16 y;
	u16 hp;
	u16 ap;
	u16 mvSpeed;
};
//对u16数据的编码与解码
#define decodeu16Data(code)			((((code) & (127 << 8)) << 1) + (((code) & 127) << 2) + 2);
#define decodeu16DataHL(H,L)		((((u16)(H))<<7)+(u16)(L))		
#define encodeu16Data(data)			((((data) & (254 << 8)) >> 1) + ((((data) >> 1) & 254) >> 1))
#define encodeu16DataH(data)		((u8)((data) >> 7))
#define encodeu16DataL(data)		((u8)((data) & 127))
//u64数据（有损压缩与解压）
void encodeu64Data(string& msg, u64 data, int startCnt) {
	data >>= 1;
	const u64 mask = 127;
	for (u32 i = 0; i < 9; ++i) {
		msg[startCnt++] = (u8)(data & mask);
		data >>= 7;
	}
}
u64 decodeu64Data(string& msg, int startCnt) {
	u64 result = 0;
	for (int i = 0; i < 9; ++i) result += (((u64)msg[startCnt++]) << (i * 7));
	result <<= 1;
	return result;
}
//对u64数据的无损压缩与解压
void encodeGUID(string& msg, u64 guid, int startCnt) {
	const u64 mask = 127;
	for (u32 i = 0; i < 10; ++i) {
		msg[startCnt + i] = (u8)(guid & mask);
		guid >>= 7;
	}
}
u64 decodeGUID(string& msg, int startCnt) {
	u64 ret = 0;
	for (int i = 0; i < 10; ++i) ret += (((u64)msg[startCnt + i]) << (i * 7));
	return ret;
}

//用于处理接受消息的全局变量
string MSG(64, 0);												//用于临时存储接收到的消息
u32 MSGCntRec;													//用于接受消息过程中对字节的计数
u32 MSGCntSend;													//发送普通型消息的计数

//消息头（id和时间两部分，总共3个字节）
namespace MSGhead {
	//消息id部分

	//获取发送者身份
	constexpr inline int LocID(u8 id) { return (id & 3); }
	//判断消息类型（Normal或Abnormal）
	constexpr inline u8 NormalMode(u8 id) { return (id & 4) >> 2; }
	//提取Abnormal类型消息的Type部分
	constexpr inline u8 AbnormalType(u8 id) { return ((id & (15 << 3)) >> 3); }
	//Abnormal类型消息的不同Type：
	constexpr inline u8 Initialize = 0;
	constexpr inline u8 CallAttack = 1;
	constexpr inline u8 ExteriorAttackInfo = 2;
	constexpr inline u8 CallColor = 3;
	constexpr inline u8 UpdatePropAlloc = 4;
	constexpr inline u8 EnemyInfo = 5;

	//Normal类型下的四个比特开关
	constexpr inline u8 UpdateCharaInfo(u8 id) { return (id & 8) >> 3; }
	constexpr inline u8 UpdateMap(u8 id) { return (id & 16) >> 4; }
	constexpr inline u8 UpdateEnemyInfo(u8 id) { return (id & 32) >> 5; }
	//生成普通消息的消息头
	constexpr inline u8 MakeIDNormal(u8 locid, u8 updatecharainfo, u8 updatemap, u8 updateenemyinfo, u8 reserved) {
		return (locid + ((u8)1 << 2) + (updatecharainfo << 3) + (updatemap << 4) + (updateenemyinfo << 5) + (reserved << 6));
	}
	//计算普通消息的长度
	constexpr inline u32 MakeMSGLenNormal(u8 updatecharainfo, u8 updatemap, u8 updateenemyinfo, u8 reserved) {
		if (updatemap)return 64;								//说不准消息得有多长
		return 3 + updatecharainfo * CharaInfoLen + reserved * 2;
	}
	//生成特殊消息的消息头
	constexpr inline u8 MakeIDAbnormal(u8 locid, u8 type) {
		return (locid + (type << 3));
	}

	//消息发送时间（2bytes）
	u16 decodeTime(string& msg) { return (((u16)msg[2]) << 7) + ((u16)msg[1]); }
	void encodeTime(string& msg);
	bool checkTime(string& msg, u32 epochDiff);					//判断消息时效性

}//namespace MSGhead

//普通消息的内容
namespace NormalMSG {

	//角色基本信息（8bytes）
	void decodeChara(string& msg, SimplifiedCharacter& chara);
	void encodeChara(string& msg, SimplifiedCharacter& chara);

	//地图更新（unknow bytes）
	void decodeMapUpdate(string& msg);
	void encodeMapUpdate(string& msg);

}//namespace NormalMSG

//非普通消息的处理
namespace AbnormalMSG {

	//游戏开始时间同步
	void SendInitMSG(GameApi& self, int IDto);
	//攻击呼叫

	//视野外打击信息同步
	void SendExterialAttackInfo(GameApi& self, int cellX, int cellY);

	//发送道具主权声明
	void DeclarePropOwnership(GameApi& self, u64 guid, u64 timeOfDiscovery);

	//敌情通报
	void SendEnemyInfo(GameApi& self, int cellX, int cellY);
}//namespace AbnormalMSG

//识别原地遍历的队伍
struct birthpoint {
	int cellX;
	int cellY;
	int discoverCnt;			//在出生点发现敌人的次数

	int lastHitTime;
	birthpoint(int cx, int cy, int t) :cellX(cx), cellY(cy), discoverCnt(0), lastHitTime(t) {  }
};
vector<birthpoint> enemyBirthPoints = {};

//关于敌人的信息
vector<u64> enemyGUIDs = {};

//关于自己及队伍的信息
u32 myLocalID = -1;									//自己的玩家ID
vector<SimplifiedCharacter> teamMates = {};			//包括自己的队伍信息
int myBirthPointX = 0, myBirthPointY = 0;			//自己的出生点cell位置

//标注不可拾取的道具，避免卡死
list<u64> unPickableProps = {};
int isPickable(u64 GUID) {
	int ret = 1;
	for (auto it = unPickableProps.begin(); it != unPickableProps.end(); ++it) {
		if ((*it) == GUID)ret = 0;
	}
	return ret;
}
void markAsUnpickable(u64 GUID) {
	unPickableProps.emplace_back(GUID);
}

//调用的入口模式
enum MonkeyDoctorModes {
	INITIALIZE,					//游戏开始时的初始化
	DEFAULT,					//默认入口
	CHARGE,						//最后总攻
};
u8 mode = INITIALIZE;			//全局变量：模式。初始化为INITIALIZE

//攻击函数
void AttackCell(GameApi& g, int CellX, int CellY);
void AttackPoint(GameApi& g, int X, int Y);

//运动函数
void MoveToPoint(GameApi& g, int X, int Y);
void MoveToCell(GameApi& g, int cellX, int cellY);
void Move(GameApi& g);		//恒为50ms，运动途中可以做别的事
double movingAngle = 0;		//与Move()配套的角度
int moveDuration = 50;		//一次运动的时间

int anyBarrier(int destinationX, int destinationY);
int getMSGs(GameApi& g);				//如果有攻击呼叫，就返回1，程序退出并处理攻击
void CallAttack(GameApi& g, int X, int Y, int enemySpeed);		//呼叫攻击
void CallColor(GameApi& g, int cellX, int cellY);
void SendNormalMSG(GameApi& g);
void ThrowToCell(GameApi& g, int cellX, int cellY);
//动态规划模块
#define PATH_PLAN_DEPTH		20						//路径规划的深度
//节点类型
struct node {
	u8 x;
	u8 y;
	u8 chosen;										//在进行清除的时候是否收到保护
	node* source;
	double distance;
	double points;
	node(int xx, int yy, double dd, double Points, node* pp) :x(xx), y(yy), distance(dd), chosen(0), source(pp), points(Points) {  }
	node() = default;
	node(const node& n) :x(n.x), y(n.y), chosen(n.chosen), source(n.source), distance(n.distance), points(n.points) {  }
	~node() {  }
};
//递归释放内存
void clearPath(node* n) {
	if (n->chosen)return;							//就不再追究了
	if (n->source == NULL) delete n;
	else clearPath(n->source);
}
void markPath(node* n, bool mark) {
	n->chosen = mark;
	if (n->source == NULL) return;
	else markPath(n->source, 1);
}
double PlanPathToCell(int start_x, int start_y, int end_x, int end_y);
double PlanPathToExplore(int start_x, int start_y);

//全局开关

//地图评分更新控制
#define MAPVALUE_DECAY_T			500
#define MAPVALUE_DECAY_RATE			0.999
#define RENEW_MAPVALUE_T			25
int lastCellX = 0, lastCellY = 0;
int lastDecayTime = 0;

//运动决策控制
#define DIRECTION_NUM				64
double angleUnit = twoPI / DIRECTION_NUM;

#define DISTANCE_DECAY_RATE			0.99986
#define DIRECTION_DECAY_RATE		0.5
#define CRITICAL_PASSABILITY		300

int lastX = 0, lastY = 0;								//用于脱困
int lastBeenHere = 0;

int ExploreTime = 1000;								//本次探索的时间：初次为5s
int TimeEnteringExploreMode = 0;						//上次进入探索模式的时间
int exploreMode = 1;

#define COLOR_DECAY_RATE			0.9

double distances[DIRECTION_NUM];						//各个方向离障碍物的距离
double safeDistances[DIRECTION_NUM];					//各个方向可以安全行驶的距离
double safeDistanceConvs[DIRECTION_NUM];				//安全距离的卷积值
double colorPoints[DIRECTION_NUM];						//根据染色情况计算的得分

int returningMode = 0;									//返回出生点的模式
int havingPropAtBirthPoint = 0;							//出生点有没有存道具

//染色控制
double CRITICAL_COLOR_VALUE = 6;
void CallAttack(GameApi& g, const THUAI4::Character enemy);
int holdingSchoolBag = 0;
int SENSE_COLOR_T = 80;								//用来感知周围的颜色
int colorMap[11][11];									//该块地的评分
int grades3Conv[11][11];								//虽然有11*11，但大部分都是用不上的

//道具分配控制
u64 targetPropGUID = 0;									//当前的目标道具的GUID
u64 timeDiscoveringTargetProp = 0;						//与目标道具的初识时间
list<u64> propsToNeglect = {};							//在探寻中需要忽视的道具
int isNeglected(u64 guid) {
	int ret = 0;
	for (auto it = propsToNeglect.begin(); it != propsToNeglect.end(); ++it) {
		if (*it == guid) {
			return true;
		}
	}
	return false;
}
void addToNeglect(u64 guid) {
	propsToNeglect.emplace_back(guid);
}
void removeFromNeglect(u64 guid) {
	propsToNeglect.remove(guid);
}

int dyingJustNow = 0;
//主调用函数
void AI::play(GameApi& g)
{

	switch (mode) {


	case INITIALIZE: {

		InitMap();															//初始化地图
		e.seed(NOW);														//设置随机数引擎（尽管没有用上）
		startTime = NOW;													//获取游戏开始的时间（毫秒）
		self = *g.GetSelfInfo();
		teamColor = g.GetSelfTeamColor();									//获取本队颜色
		lastX = self.x, lastY = self.y;
		TimeEnteringExploreMode = TIME_SINCE_START;
		lastBeenHere = TIME_SINCE_START;
		myBirthPointX = self.x / 1000;										//自己的出生点（不算做障碍物）
		myBirthPointY = self.y / 1000;

		//初始化运动角度
		if (myBirthPointX == 3 && myBirthPointY == 2)movingAngle = 0;
		else if (myBirthPointX == 2 && myBirthPointY == 3)movingAngle = PI / 2;
		else if (myBirthPointX == 2 && myBirthPointY == 46)movingAngle = 3 * PI / 2;
		else if (myBirthPointX == 3 && myBirthPointY == 47)movingAngle = 0;
		else if (myBirthPointX == 46 && myBirthPointY == 2)movingAngle = PI;
		else if (myBirthPointX == 47 && myBirthPointY == 3)movingAngle = PI / 2;
		else if (myBirthPointX == 46 && myBirthPointY == 47)movingAngle = PI;
		else if (myBirthPointX == 47 && myBirthPointY == 46)movingAngle = 3 * PI / 2;

		

		//确认游戏规模，并获取自己的LocalID
		auto& GUIDs = g.GetPlayerGUIDs();
		for (u32 cnt = 0; cnt < PlayerNum; ++cnt) {
			if (GUIDs[self.teamID][cnt] == self.guid) {
				myLocalID = cnt;
				SimplifiedCharacter newChara(self);		//在创建时就把数据编码好了
				teamMates.push_back(newChara);
			}
			SimplifiedCharacter newChara;
			teamMates.push_back(newChara);
		}
		teamMates[myLocalID].upDate(self);				//更新用于发送的自身信息

		//向队友发送初始化消息
		for (int cnt = 0; cnt < PlayerNum; ++cnt) {
			if (cnt != myLocalID) {
				//AbnormalMSG::SendInitMSG(g, cnt);
			}
		}
		mode = DEFAULT;
		break;
	}//case INITIALIZE


	case DEFAULT: {
		while (true) {

			teamScore = g.GetTeamScore();
			self = *g.GetSelfInfo();
			int cellX = self.x / 1000, cellY = self.y / 1000;
			if (moveDuration != 50)moveDuration = 50;

			if (self.isDying) {
				//初始化运动角度
				dyingJustNow = 1;
				if (myBirthPointX == 3 && myBirthPointY == 2)movingAngle = 0;
				else if (myBirthPointX == 2 && myBirthPointY == 3)movingAngle = PI / 2;
				else if (myBirthPointX == 2 && myBirthPointY == 46)movingAngle = 3 * PI / 2;
				else if (myBirthPointX == 3 && myBirthPointY == 47)movingAngle = 0;
				else if (myBirthPointX == 46 && myBirthPointY == 2)movingAngle = PI;
				else if (myBirthPointX == 47 && myBirthPointY == 3)movingAngle = PI / 2;
				else if (myBirthPointX == 46 && myBirthPointY == 47)movingAngle = PI;
				else if (myBirthPointX == 47 && myBirthPointY == 46)movingAngle = 3 * PI / 2;
				exploreMode = true;
				ExploreTime = 3000;
				TimeEnteringExploreMode = TIME_SINCE_START;
				g.Wait();
			}
			else {
				if (dyingJustNow == 1) {
					dyingJustNow = 0;
					for (int i = 0; i < 5; ++i) {
						g.Wait();
						Move(g);
					}
				}
			}
			
			//遍历该打的人
			for (auto it = enemyBirthPoints.begin(); it != enemyBirthPoints.end(); ++it) {
				if (it->discoverCnt >= 3 && TIME_SINCE_START - it->lastHitTime >= 40000) {
					for (int i = 0; i < 5; ++i) {
						g.Wait();
						AttackCell(g, it->cellX, it->cellY);
						it->lastHitTime = TIME_SINCE_START;
					}
				}
			}

			if (self.bulletNum == self.maxBulletNum) {
				//打视野外的地方
				int targetX = -1, targetY = -1;
				if (self.teamID == 0) {
					int maxTime = 0;
					//从下往上搜索
					for (int cnt_y = 2; cnt_y <= 47; cnt_y += 3) {
						for (int cnt_x = 47; cnt_x >= 23; cnt_x -= 3) {
							if (TIME_SINCE_START - lastExplorationTime[cnt_x][cnt_y] > maxTime) {
								maxTime = TIME_SINCE_START - lastExplorationTime[cnt_x][cnt_y];
								targetX = cnt_x;
								targetY = cnt_y;
							}
						}
					}
				}
				else {
					int maxTime = 0;
					//从下往上搜索
					for (int cnt_y = 2; cnt_y <= 47; cnt_y += 3) {
						for (int cnt_x = 2; cnt_x <= 29; cnt_x += 3) {
							if (TIME_SINCE_START - lastExplorationTime[cnt_x][cnt_y] > maxTime) {
								maxTime = TIME_SINCE_START - lastExplorationTime[cnt_x][cnt_y];
								targetX = cnt_x;
								targetY = cnt_y;
							}
						}
					}
				}
				g.Wait();
				AttackCell(g, targetX, targetY);
				lastExplorationTime[targetX][targetY] = TIME_SINCE_START;
				AbnormalMSG::SendExterialAttackInfo(g, cellX, cellY);
				//打完了通知队友
			}

//确定攻击强度
			if (teamScore > 1000)CRITICAL_COLOR_VALUE = 5;
			else if (teamScore > 800)CRITICAL_COLOR_VALUE = 6;
			else if (teamScore > 300)CRITICAL_COLOR_VALUE = 7;
			else CRITICAL_COLOR_VALUE = 8;

//紧急脱困A（向无法前进的方向运动，即卡死）
			if (self.isDying)lastBeenHere = TIME_SINCE_START;
			double distanceToLastPosition = sqrt(((double)self.x - (double)lastX) * ((double)self.x - (double)lastX) + ((double)self.y - (double)lastY) * ((double)self.y - (double)lastY));
			if (distanceToLastPosition > 150) {
				lastX = self.x, lastY = self.y;
				lastBeenHere = TIME_SINCE_START;
			}
			else if (TIME_SINCE_START - lastBeenHere > 2000) {			//1.5秒没有移动，表示被困住了
				movingAngle += PI;										//掉头
				if (movingAngle >= twoPI)movingAngle -= twoPI;

				exploreMode = true;
				ExploreTime = 1000;										//探索模式两秒，脱离卡死
				TimeEnteringExploreMode = TIME_SINCE_START;
				lastBeenHere = TIME_SINCE_START;
			}
			//cout << movingAngle << endl;
//更新本地墙壁信息
			WALLS_PTR visibleWalls = g.GetWalls();
			updateLocalMap(visibleWalls);

//感知障碍物
			for (int i = 0; i < DIRECTION_NUM; ++i) {				//初始化
				safeDistances[i] = 5000;
				safeDistanceConvs[i] = 0;
				distances[i] = 7000;								//表示视野内该方向没有发现障碍物
				colorPoints[i] = 0;
			}
			//计算出distances
			auto visibleBirthPoints = g.GetBirthPoints();
			auto visibleCharacters = g.GetCharacters();
			for (auto it = visibleWalls.begin(); it != visibleWalls.end(); ++it) {
				//计算出这块墙壁所在的方位和距离
				double diffX = (int)((*it)->x) - (int)(self.x);
				double diffY = (int)((*it)->y) - (int)(self.y);
				double angle = atan(diffY / diffX);
				if (diffX < 0)angle += PI;
				else if (diffY < 0)angle += twoPI;
				double distance = sqrt(diffX * diffX + diffY * diffY);

				//计算出这块墙壁所张的角度上下界
				double minDistance = distance, minAngleDistance = distance, maxAngleDistance = distance;
				double maxAngle = 0, minAngle = twoPI, minDistanceAngle = 0;
				if (((diffY >= -500) && (diffY < 500)) && diffX > 0) {
					for (int i = -1; i <= 1; i += 2) {
						for (int j = -1; j <= 1; j += 2) {
							double cornor_diffX = (int)((*it)->x + i * 500) - (int)(self.x);
							double cornor_diffY = (int)((*it)->y + j * 500) - (int)(self.y);
							double cornor_angle = atan(cornor_diffY / cornor_diffX);
							if (cornor_diffX < 0)cornor_angle += PI;
							double cornor_distance = sqrt(cornor_diffX * cornor_diffX + cornor_diffY * cornor_diffY);
							if (cornor_angle < minAngle) {									//获取最小角度	
								minAngle = cornor_angle;
								minAngleDistance = cornor_distance;
							}
							if (cornor_angle > maxAngle) {									//获取最大角度
								maxAngle = cornor_angle;
								maxAngleDistance = cornor_distance;
							}
							if (cornor_distance < minDistance) {							//获取距离最小对应的角度
								minDistanceAngle = cornor_angle;
								minDistance = cornor_distance;
							}
						}
					}
					minAngle += twoPI;
					minDistanceAngle += twoPI;
					maxAngle += twoPI;
				}
				else {
					for (int i = -1; i <= 1; i += 2) {
						for (int j = -1; j <= 1; j += 2) {
							double cornor_diffX = (int)((*it)->x + i * 500) - (int)(self.x);
							double cornor_diffY = (int)((*it)->y + j * 500) - (int)(self.y);
							double cornor_angle = atan(cornor_diffY / cornor_diffX);
							if (cornor_diffX < 0)cornor_angle += PI;
							else if (cornor_diffY < 0)cornor_angle += twoPI;
							double cornor_distance = sqrt(cornor_diffX * cornor_diffX + cornor_diffY * cornor_diffY);
							if (cornor_angle < minAngle) {									//获取最小角度	
								minAngle = cornor_angle;
								minAngleDistance = cornor_distance;
							}
							if (cornor_angle > maxAngle) {									//获取最大角度
								maxAngle = cornor_angle;
								maxAngleDistance = cornor_distance;
							}
							if (cornor_distance < minDistance) {							//获取距离最小对应的角度
								minDistanceAngle = cornor_angle;
								minDistance = cornor_distance;
							}
						}
					}
				}
				int minDir = minAngle / angleUnit;
				int maxDir = maxAngle / angleUnit;

				for (int i = minDir + 1; i <= maxDir; ++i) {
					double cur_angle = i * angleUnit;							//当前操作的角度
					double distanceOfThisDir = 0;
					if (cur_angle < minDistanceAngle) {							//前半段
						double theta_1 = cur_angle - minAngle, theta_2 = minDistanceAngle - cur_angle;
						double fact_1 = minAngleDistance * sin(theta_1), fact_2 = minDistance * sin(theta_2);
						distanceOfThisDir = (minDistance + minAngleDistance) * (minDistance - minAngleDistance) - 1000 * 1000 * (fact_2 - fact_1) / (fact_1 + fact_2);
						distanceOfThisDir /= (2 * (minDistance * cos(theta_2) - minAngleDistance * cos(theta_1)));
					}
					else {														//后半段
						double theta_1 = cur_angle - minDistanceAngle, theta_2 = maxAngle - cur_angle;
						double fact_1 = minDistance * sin(theta_1), fact_2 = maxAngleDistance * sin(theta_2);
						distanceOfThisDir = (maxAngleDistance + minDistance) * (maxAngleDistance - minDistance) - 1000 * 1000 * (fact_2 - fact_1) / (fact_2 + fact_1);
						distanceOfThisDir /= (2 * (maxAngleDistance * cos(theta_2) - minDistance * cos(theta_1)));
					}
					if (distanceOfThisDir < distances[i % DIRECTION_NUM])distances[i % DIRECTION_NUM] = distanceOfThisDir;
				}
			}//for (auto it = visibleWalls.begin(); it != visibleWalls.end(); ++it)
			for (auto it = visibleBirthPoints.begin(); it != visibleBirthPoints.end(); ++it) {
				if ((*it)->x / 1000 == myBirthPointX && (*it)->y / 1000 == myBirthPointY)continue;
				//计算出这块墙壁所在的方位和距离
				double diffX = (int)((*it)->x) - (int)(self.x);
				double diffY = (int)((*it)->y) - (int)(self.y);
				double angle = atan(diffY / diffX);
				if (diffX < 0)angle += PI;
				else if (diffY < 0)angle += twoPI;
				double distance = sqrt(diffX * diffX + diffY * diffY);
				// cout << "=====wall=====\n";

				//计算出这块墙壁所张的角度上下界
				double minDistance = distance, minAngleDistance = distance, maxAngleDistance = distance;
				double maxAngle = 0, minAngle = twoPI, minDistanceAngle = 0;
				if (((diffY >= -500) && (diffY < 500)) && diffX > 0) {
					for (int i = -1; i <= 1; i += 2) {
						for (int j = -1; j <= 1; j += 2) {
							double cornor_diffX = (int)((*it)->x + i * 500) - (int)(self.x);
							double cornor_diffY = (int)((*it)->y + j * 500) - (int)(self.y);
							double cornor_angle = atan(cornor_diffY / cornor_diffX);
							if (cornor_diffX < 0)cornor_angle += PI;
							double cornor_distance = sqrt(cornor_diffX * cornor_diffX + cornor_diffY * cornor_diffY);

							if (cornor_angle < minAngle) {									//获取最小角度	
								minAngle = cornor_angle;
								minAngleDistance = cornor_distance;
							}
							if (cornor_angle > maxAngle) {									//获取最大角度
								maxAngle = cornor_angle;
								maxAngleDistance = cornor_distance;
							}
							if (cornor_distance < minDistance) {							//获取距离最小对应的角度
								minDistanceAngle = cornor_angle;
								minDistance = cornor_distance;
							}
						}
					}
					minAngle += twoPI;
					minDistanceAngle += twoPI;
					maxAngle += twoPI;
				}
				else {
					for (int i = -1; i <= 1; i += 2) {
						for (int j = -1; j <= 1; j += 2) {
							double cornor_diffX = (int)((*it)->x + i * 500) - (int)(self.x);
							double cornor_diffY = (int)((*it)->y + j * 500) - (int)(self.y);
							double cornor_angle = atan(cornor_diffY / cornor_diffX);
							if (cornor_diffX < 0)cornor_angle += PI;
							else if (cornor_diffY < 0)cornor_angle += twoPI;
							double cornor_distance = sqrt(cornor_diffX * cornor_diffX + cornor_diffY * cornor_diffY);

							if (cornor_angle < minAngle) {									//获取最小角度	
								minAngle = cornor_angle;
								minAngleDistance = cornor_distance;
							}
							if (cornor_angle > maxAngle) {									//获取最大角度
								maxAngle = cornor_angle;
								maxAngleDistance = cornor_distance;
							}
							if (cornor_distance < minDistance) {							//获取距离最小对应的角度
								minDistanceAngle = cornor_angle;
								minDistance = cornor_distance;
							}
						}
					}
				}
				int minDir = minAngle / angleUnit;
				int maxDir = maxAngle / angleUnit;
				for (int i = minDir + 1; i <= maxDir; ++i) {
					double cur_angle = i * angleUnit;							//当前操作的角度
					double distanceOfThisDir = 0;
					if (cur_angle < minDistanceAngle) {							//前半段
						double theta_1 = cur_angle - minAngle, theta_2 = minDistanceAngle - cur_angle;
						double fact_1 = minAngleDistance * sin(theta_1), fact_2 = minDistance * sin(theta_2);
						distanceOfThisDir = (minDistance + minAngleDistance) * (minDistance - minAngleDistance) - 1000 * 1000 * (fact_2 - fact_1) / (fact_1 + fact_2);
						distanceOfThisDir /= (2 * (minDistance * cos(theta_2) - minAngleDistance * cos(theta_1)));
					}
					else {														//后半段
						double theta_1 = cur_angle - minDistanceAngle, theta_2 = maxAngle - cur_angle;
						double fact_1 = minDistance * sin(theta_1), fact_2 = maxAngleDistance * sin(theta_2);
						distanceOfThisDir = (maxAngleDistance + minDistance) * (maxAngleDistance - minDistance) - 1000 * 1000 * (fact_2 - fact_1) / (fact_2 + fact_1);
						distanceOfThisDir /= (2 * (maxAngleDistance * cos(theta_2) - minDistance * cos(theta_1)));
					}
					if (distanceOfThisDir < distances[i % DIRECTION_NUM])distances[i % DIRECTION_NUM] = distanceOfThisDir;
				}
			}//for (auto it = visibleWalls.begin(); it != visibleWalls.end(); ++it)
			for (auto it = visibleCharacters.begin(); it != visibleCharacters.end(); ++it) {
				if ((*it)->guid == self.guid)continue;
				//计算出这块墙壁所在的方位和距离
				double diffX = (int)((*it)->x) - (int)(self.x);
				double diffY = (int)((*it)->y) - (int)(self.y);
				double angle = atan(diffY / diffX);
				if (diffX < 0)angle += PI;
				else if (diffY < 0)angle += twoPI;
				double distance = sqrt(diffX * diffX + diffY * diffY);

				//计算出这块墙壁所张的角度上下界
				double minDistance = distance, minAngleDistance = distance, maxAngleDistance = distance;
				double maxAngle = 0, minAngle = twoPI, minDistanceAngle = 0;
				if (((diffY >= -500) && (diffY < 500)) && diffX > 0) {
					for (int i = -1; i <= 1; i += 2) {
						for (int j = -1; j <= 1; j += 2) {
							double cornor_diffX = (int)((*it)->x + i * 500) - (int)(self.x);
							double cornor_diffY = (int)((*it)->y + j * 500) - (int)(self.y);
							double cornor_angle = atan(cornor_diffY / cornor_diffX);
							if (cornor_diffX < 0)cornor_angle += PI;
							double cornor_distance = sqrt(cornor_diffX * cornor_diffX + cornor_diffY * cornor_diffY);
							if (cornor_angle < minAngle) {									//获取最小角度	
								minAngle = cornor_angle;
								minAngleDistance = cornor_distance;
							}
							if (cornor_angle > maxAngle) {									//获取最大角度
								maxAngle = cornor_angle;
								maxAngleDistance = cornor_distance;
							}
							if (cornor_distance < minDistance) {							//获取距离最小对应的角度
								minDistanceAngle = cornor_angle;
								minDistance = cornor_distance;
							}
						}
					}
					minAngle += twoPI;
					minDistanceAngle += twoPI;
					maxAngle += twoPI;
				}
				else {
					for (int i = -1; i <= 1; i += 2) {
						for (int j = -1; j <= 1; j += 2) {
							double cornor_diffX = (int)((*it)->x + i * 500) - (int)(self.x);
							double cornor_diffY = (int)((*it)->y + j * 500) - (int)(self.y);
							double cornor_angle = atan(cornor_diffY / cornor_diffX);
							if (cornor_diffX < 0)cornor_angle += PI;
							else if (cornor_diffY < 0)cornor_angle += twoPI;
							double cornor_distance = sqrt(cornor_diffX * cornor_diffX + cornor_diffY * cornor_diffY);

							if (cornor_angle < minAngle) {									//获取最小角度	
								minAngle = cornor_angle;
								minAngleDistance = cornor_distance;
							}
							if (cornor_angle > maxAngle) {									//获取最大角度
								maxAngle = cornor_angle;
								maxAngleDistance = cornor_distance;
							}
							if (cornor_distance < minDistance) {							//获取距离最小对应的角度
								minDistanceAngle = cornor_angle;
								minDistance = cornor_distance;
							}
						}
					}
				}
				int minDir = minAngle / angleUnit;
				int maxDir = maxAngle / angleUnit;
				for (int i = minDir + 1; i <= maxDir; ++i) {
					double cur_angle = i * angleUnit;							//当前操作的角度
					double distanceOfThisDir = 0;
					if (cur_angle < minDistanceAngle) {							//前半段
						double theta_1 = cur_angle - minAngle, theta_2 = minDistanceAngle - cur_angle;
						double fact_1 = minAngleDistance * sin(theta_1), fact_2 = minDistance * sin(theta_2);
						distanceOfThisDir = (minDistance + minAngleDistance) * (minDistance - minAngleDistance) - 1000 * 1000 * (fact_2 - fact_1) / (fact_1 + fact_2);
						distanceOfThisDir /= (2 * (minDistance * cos(theta_2) - minAngleDistance * cos(theta_1)));
					}
					else {														//后半段
						double theta_1 = cur_angle - minDistanceAngle, theta_2 = maxAngle - cur_angle;
						double fact_1 = minDistance * sin(theta_1), fact_2 = maxAngleDistance * sin(theta_2);
						distanceOfThisDir = (maxAngleDistance + minDistance) * (maxAngleDistance - minDistance) - 1000 * 1000 * (fact_2 - fact_1) / (fact_2 + fact_1);
						distanceOfThisDir /= (2 * (maxAngleDistance * cos(theta_2) - minDistance * cos(theta_1)));
					}
					if (distanceOfThisDir < distances[i % DIRECTION_NUM])distances[i % DIRECTION_NUM] = distanceOfThisDir;
				}
			}//for (auto it = visibleWalls.begin(); it != visibleWalls.end(); ++it)

//计算出Safedistances
			for (int i = 0; i < DIRECTION_NUM; ++i) {
				double angleUnit = twoPI / DIRECTION_NUM;
				double cur_angle = i * angleUnit;
				//找出循环的上界和下界
				double lower_angle = cur_angle - PI / 2, upper_angle = cur_angle + PI / 2;
				int lower_cnt = i - DIRECTION_NUM / 4, upper_cnt = i + DIRECTION_NUM / 4;
				if (lower_cnt >= 0 && upper_cnt < DIRECTION_NUM) {							//正常情况
					for (int j = lower_cnt; j <= upper_cnt; ++j) {
						double theta = abs(cur_angle - j * angleUnit);				//与行进方向的夹角
						double d = abs(distances[j] * sin(theta));
						if (d <= 500) {						//是个障碍
							double barrierDistance = abs(distances[j] * cos(theta)) - sqrt(500 * 500 - d * d);
							if (barrierDistance < safeDistances[i] && barrierDistance >= 0)safeDistances[i] = barrierDistance;
						}
					}
				}
				if (lower_cnt < 0) {
					lower_cnt += DIRECTION_NUM;
					for (int j = lower_cnt; j < DIRECTION_NUM; ++j) {
						double theta = abs(cur_angle - j * angleUnit);				//与行进方向的夹角
						double d = abs(distances[j] * sin(theta));
						if (d <= 500) {						//是个障碍
							double barrierDistance = abs(distances[j] * cos(theta)) - sqrt(500 * 500 - d * d);
							if (barrierDistance < safeDistances[i] && barrierDistance >= 0)safeDistances[i] = barrierDistance;
						}
					}
					for (int j = 0; j <= upper_cnt; ++j) {
						double theta = abs(cur_angle - j * angleUnit);				//与行进方向的夹角
						double d = abs(distances[j] * sin(theta));
						if (d <= 500) {						//是个障碍
							double barrierDistance = abs(distances[j] * cos(theta)) - sqrt(500 * 500 - d * d);
							if (barrierDistance < safeDistances[i] && barrierDistance >= 0)safeDistances[i] = barrierDistance;
						}
					}
				}//if (lower_cnt < 0)
				if (upper_cnt >= DIRECTION_NUM) {
					upper_cnt -= DIRECTION_NUM;
					for (int j = lower_cnt; j < DIRECTION_NUM; ++j) {
						double theta = abs(cur_angle - j * angleUnit);				//与行进方向的夹角
						double d = abs(distances[j] * sin(theta));
						if (d <= 500) {						//是个障碍
							double barrierDistance = abs(distances[j] * cos(theta)) - sqrt(500 * 500 - d * d);
							if (barrierDistance < safeDistances[i] && barrierDistance >= 0)safeDistances[i] = barrierDistance;
						}
					}
					for (int j = 0; j <= upper_cnt; ++j) {
						double theta = abs(cur_angle - j * angleUnit);				//与行进方向的夹角
						double d = abs(distances[j] * sin(theta));
						if (d <= 500) {						//是个障碍
							double barrierDistance = abs(distances[j] * cos(theta)) - sqrt(500 * 500 - d * d);
							if (barrierDistance < safeDistances[i] && barrierDistance >= 0)safeDistances[i] = barrierDistance;
						}
					}
				}//if (upper_cnt >= DIRECTION_NUM)
			}//for (int i = 0; i < DIRECTION_NUM; ++i)

//对safeDistances进行卷积
			int passable[DIRECTION_NUM];
			for (int i = 0; i < DIRECTION_NUM; ++i) {
				int start = i - 1, end = i + 1;
				passable[i] = 1;
				if (start >= 0 && end < DIRECTION_NUM) {
					for (int j = start; j <= end; ++j) {
						if (passable[i] == 1 && safeDistances[j] < CRITICAL_PASSABILITY)passable[i] = 0;
						safeDistanceConvs[i] += safeDistances[j];
					}
					safeDistanceConvs[i] /= 5;
				}
				else if (start < 0) {
					for (int j = start + DIRECTION_NUM; j < DIRECTION_NUM; ++j) {
						if (passable[i] == 1 && safeDistances[j] < CRITICAL_PASSABILITY)passable[i] = 0;
						safeDistanceConvs[i] += safeDistances[j];
					}
					for (int j = 0; j <= end; ++j) {
						if (passable[i] == 1 && safeDistances[j] < CRITICAL_PASSABILITY)passable[i] = 0;
						safeDistanceConvs[i] += safeDistances[j];
					}
					safeDistanceConvs[i] /= 5;
				}
				else if (end >= DIRECTION_NUM) {
					for (int j = start; j < DIRECTION_NUM; ++j) {
						if (passable[i] == 1 && safeDistances[j] < CRITICAL_PASSABILITY)passable[i] = 0;
						safeDistanceConvs[i] += safeDistances[j];
					}
					end -= DIRECTION_NUM;
					for (int j = 0; j <= end; ++j) {
						if (passable[i] == 1 && safeDistances[j] < CRITICAL_PASSABILITY)passable[i] = 0;
						safeDistanceConvs[i] += safeDistances[j];
					}
					safeDistanceConvs[i] /= 5;
				}
			}//for (int i = 0; i < DIRECTION_NUM; ++i) 
//感知其他角色
			for (auto it = visibleCharacters.begin(); it != visibleCharacters.end(); ++it) {
				if ((*it)->teamID == self.teamID) {					//队友
					//类似对子弹的感知：要感知到所有的角色。不能相撞，按照当前算法，只要挤在一起程序就完了。
					//策略：首先检测当前移动方向下的撞击风险。如果存在，就把当前方向即偏左的safeDistance设为0.

					//PI以内的运动角度差
					double theta = movingAngle - (*it)->facingDirection;
					if (theta > PI)theta -= twoPI;
					if (theta < -PI)theta += twoPI;

					//计算出该角色相对自己的运动方向
					double psi = (((double)self.moveSpeed) * sin(theta)) / (((double)((*it)->moveSpeed)) - ((double)self.moveSpeed) * cos(theta));
					psi = (*it)->facingDirection - psi;
					if (psi > PI)psi -= twoPI;
					if (psi < -PI)psi += twoPI;

					//计算出该角色相对自己的运动速度
					theta = PI - abs(theta);
					double relativeVel = (double)(((*it)->moveSpeed) * ((*it)->moveSpeed)) + (double)(self.moveSpeed * self.moveSpeed) - 2 * ((double)(((*it)->moveSpeed) * self.moveSpeed)) * cos(theta);
					relativeVel = sqrt(relativeVel);

					//计算出该角色相对自己在径向靠近（远离）和横向靠近（远离）的速度：以远离为正方向
					double objDir = atan((((double)((*it)->y)) - ((double)self.y)) / (((double)((*it)->x)) - ((double)self.x)));
					if (((double)((*it)->x)) - ((double)self.x) < 0)objDir += PI;
					else if (((double)((*it)->y)) - ((double)self.y))objDir += twoPI;

					double relativeDir = psi - objDir;
					if (relativeDir > PI)relativeDir -= twoPI;
					if (relativeDir < -PI)relativeDir += twoPI;

					double radialVel = relativeVel * cos(relativeDir);				//正方向为远离
					//double normalVel = relativeDir * sin(relativeDir);				//正方向为逆时针

					//判断是否有相撞风险
					double willClash = 0;
					double distanceToChara = sqrt((((double)((*it)->y)) - ((double)self.y)) * (((double)((*it)->y)) - ((double)self.y)) + (((double)((*it)->x)) - ((double)self.x)) * (((double)((*it)->x)) - ((double)self.x)));
					if (radialVel < 0 && abs(distanceToChara * tan(objDir / 2)) < 520) {
						//表示有相撞风险，对safeDistances进行处理。
						if (relativeDir > 0) {
							//表示应该左转，即将movingAngle增大
							int dirsToHandle = 2 * relativeDir / angleUnit;					//应该标注的方向数
							int curDir = movingAngle / angleUnit;
							int minDir = curDir - dirsToHandle;
							if (minDir >= 0) {
								for (int i = minDir; i <= curDir; ++i) {
									safeDistances[i] = 0;
								}
							}
							else {
								minDir += twoPI;
								for (int i = minDir; i < DIRECTION_NUM; ++i) {
									safeDistances[i] = 0;
								}
								for (int i = 0; i <= curDir; ++i) {
									safeDistances[i] = 0;
								}
							}
						}//if (relativeDir > 0)
						else {
							//表示应该右转
							int dirsToHandle = 2 * relativeDir / angleUnit;					//应该标注的方向数
							int curDir = movingAngle / angleUnit;
							int maxDir = curDir + dirsToHandle;
							if (maxDir < DIRECTION_NUM) {
								for (int i = curDir; i <= maxDir; ++i) {
									safeDistances[i] = 0;
								}
							}
							else {
								maxDir -= twoPI;
								for (int i = curDir; i < DIRECTION_NUM; ++i) {
									safeDistances[i] = 0;
								}
								for (int i = 0; i <= maxDir; ++i) {
									safeDistances[i] = 0;
								}
							}
						}//else
					}//if (abs(distanceToChara * tan(objDir / 2)) > 520)
				}//if ((*it)->teamID == self.teamID)
				else {
					////对于敌人
					//对于敌人
					if (!(*it)->isDying && TIME_SINCE_START - lastAttackTimes[(*it)->x / 1000][(*it)->y / 1000] > 800) {
						//首先判断是不是在出生点遍历
						int enemyCellX = (*it)->x / 1000, enemyCellY = (*it)->y / 1000;
						if ((enemyCellX == 3 && enemyCellY == 2) ||
							(enemyCellX == 2 && enemyCellY == 3) ||
							(enemyCellX == 2 && enemyCellY == 46) ||
							(enemyCellX == 3 && enemyCellY == 47) ||
							(enemyCellX == 46 && enemyCellY == 2) ||
							(enemyCellX == 47 && enemyCellY == 3) ||
							(enemyCellX == 46 && enemyCellY == 47) ||
							(enemyCellX == 47 && enemyCellY == 46)) {
							//确实是在出生点发现了敌人，先寻找是不是已经添加的
							int exists = 0;
							for (auto it = enemyBirthPoints.begin(); it != enemyBirthPoints.end(); ++it) {
								if (it->cellX == enemyCellX && it->cellY == enemyCellY) {
									exists = 1;
									++(it->discoverCnt);
									break;
								}
							}
							if (exists == 0) {
								//没发现
								birthpoint newpoint(enemyCellX, enemyCellY, TIME_SINCE_START);
								enemyBirthPoints.emplace_back(newpoint);
							}
						}
						//击毙需要的子弹数量
						int numOfBullets = (double)((*it)->hp) / (double)(self.ap) + 1;
						//发动攻击
						g.Wait();
						if (numOfBullets <= self.bulletNum - 1) {
							//如果子弹充足，则自行解决敌人（给自己留一发子弹用来回血（不是自尽））
							for (int bulletCnt = 0; bulletCnt < numOfBullets; ++bulletCnt) {
								AttackPoint(g, (*it)->x, (*it)->y);
								g.Wait();
								lastAttackTimes[(*it)->x / 1000][(*it)->y / 1000] = TIME_SINCE_START;
							}
						}
						else {
							//还差多少子弹
							int remainingHP = (*it)->hp - self.ap * self.bulletNum;

							for (int bulletCnt = 0; bulletCnt < self.bulletNum; ++bulletCnt) {
								AttackPoint(g, (*it)->x, (*it)->y);
								g.Wait();
								lastAttackTimes[(*it)->x / 1000][(*it)->y / 1000] = TIME_SINCE_START;
							}
							CallAttack(g, *(*it));
						}

						//杀人不管有没有得手都要马上跑路，通过引导角色进入explore mode来开始一段时间的探索。既然不敢动safeDistances，就动colorPoints
						//逃跑的方向
						double escapeAngle = atan(((double)self.y - (double)(*it)->y) / ((double)self.x - (double)(*it)->x));
						if (((double)self.x - (double)(*it)->x) < 0)escapeAngle += PI;
						else if (((double)self.y - (double)(*it)->y) < 0)escapeAngle += twoPI;

						//进入explore mode
						exploreMode = true;
						TimeEnteringExploreMode = TIME_SINCE_START;
						ExploreTime = 500;
						movingAngle = escapeAngle;
					}
				}
			}

			//更新地图评分
			int LimitXD = max(cellX - radiusInCells, 0),
				LimitXU = min(cellX + radiusInCells, 49),
				LimitYD = max(cellY - radiusInCells, 0),
				LimitYU = min(cellY + radiusInCells, 49);
			if (cellX != lastCellX || cellY != lastCellY) {
				lastCellX = cellX;
				lastCellY = cellY;
				//确定上下界
				for (int cnt_x = LimitXD; cnt_x <= LimitXU; ++cnt_x) {
					int y_start = LimitYD;
					while (pow(y_start - cellY, 2) + pow(cnt_x - cellX, 2) > radiusInCellsSquared) ++y_start;
					int y_end = LimitYU;
					while (pow(y_end - cellY, 2) + pow(cnt_x - cellX, 2) > radiusInCellsSquared) --y_end;
					for (int cnt_y = y_start; cnt_y <= y_end; ++cnt_y) {
						//对视野中的每一个cell循环
						if (isSelfExplored(cnt_x, cnt_y)) {
							//MapValue[cnt_x][cnt_y] += (1 - exp(-(double)(TIME_SINCE_START - lastExplorationTime[cnt_x][cnt_y]) / 5000));
							lastExplorationTime[cnt_x][cnt_y] = TIME_SINCE_START;
						}
						else {
							if (!isExplored(cnt_x, cnt_y))ExploreCell(cnt_x, cnt_y);
							SelfExploreCell(cnt_x, cnt_y);
							//MapValue[cnt_x][cnt_y] = 1;										//自己是第一次到达这个点，给初始化为1（以后每次加上的值都比1小）
							lastExplorationTime[cnt_x][cnt_y] = TIME_SINCE_START;
						}
					}//for (int cnt_y = y_start; cnt_y <= y_end; ++cnt_y)
				}//for (int cnt_x = LimitXD; cnt_x <= LimitXU; ++cnt_x)

				
			}//if (cellX != lastCellX || cellY != lastCellY)
			
			//感知周围染色状态
			for (int i = 0; i < 11; ++i) {
				for (int j = 0; j < 11; ++j) {
					colorMap[i][j] = 0;
					grades3Conv[i][j] = 0;
				}
			}
			for (int cnt_x = LimitXD; cnt_x <= LimitXU; ++cnt_x) {
				for (int cnt_y = LimitYD; cnt_y <= LimitYU; ++cnt_y) {
					if (isEmpty(cnt_x, cnt_y)) {
						auto color = g.GetCellColor(cnt_x, cnt_y);
						if (color == THUAI4::ColorType::None) {																	//发现未染色的区域
							colorMap[cnt_x - cellX + 5][cnt_y - cellY + 5] = 1;
						}
						if (color != THUAI4::ColorType::None && color != THUAI4::ColorType::Invisible && color != teamColor) {	//发现敌方染色区域
							colorMap[cnt_x - cellX + 5][cnt_y - cellY + 5] = 1;
						}
					}
				}
			}
//卷积：寻找附近最值得攻击的坐标
			int start_x = LimitXD + 1 - cellX + 5,
				end_x = LimitXU - 1 - cellX + 5,
				start_y = LimitYD + 1 - cellY + 5,
				end_y = LimitYU - 1 - cellY + 5;
			if (end_x >= start_x && end_y >= start_y) {

				for (int i = start_x; i <= end_x; ++i) {
					for (int j = start_y; j <= end_y; ++j) {
						for (int k = -1; k <= 1; ++k) {
							for (int l = -1; l <= 1; ++l) {
								grades3Conv[i][j] += colorMap[i + k][j + l];
							}
						}
					}//for (int j = start_y; j <= end_y; ++j)
				}//for (int i = start_x; i <= end_x; ++i)

				int maxConv = 0;
				int targetX = -1, targetY = -1;
				for (int i = start_x; i <= end_x; ++i) {
					for (int j = start_y; j <= end_y; ++j) {
						bool Consider = grades3Conv[i][j] > CRITICAL_COLOR_VALUE && grades3Conv[i][j] > maxConv;
						//特殊待遇1：自身位置
						if (i == 5 && j == 5 && grades3Conv[i][j] != 0) {
							Consider = 1;
						}
						//特殊待遇2：两个容易卡死的地方
						if ((i - 5 + cellX == 14 && j - 5 + cellY == 47) || (i - 5 + cellX == 35 && j - 5 + cellY == 47)) {
							if (grades3Conv[i][j] != 0)Consider = 1;
						}
						if (Consider) {											//如果第一次判断失败，就不用第二次判断了
							for (int k = -1; k <= 1; ++k) {
								for (int l = -1; l <= 1; ++l) {
									if (TIME_SINCE_START - lastAttackTimes[i + k - 5 + cellX][j + l - 5 + cellY] < 3000) {
										Consider = 0;
									}
									if (!isEmpty(i + k - 5 + cellX, j + l - 5 + cellY)) {
										Consider = 0;
									}
								}
							}
						}
						
						if (Consider) {											//如果第二次判断成功了，才进行最大值的更新
							maxConv = grades3Conv[i][j];
							targetX = i - 5 + cellX;
							targetY = j - 5 + cellY;
						}
					}//for (int j = start_y; j <= end_y; ++j)
				}//for (int i = start_x; i <= end_x; ++i)

				if (targetX != -1 && targetY != -1) {
					lastAttackTimes[targetX][targetY] = TIME_SINCE_START;
					if (self.bulletNum == 0) {
						//没有子弹的情况，就呼叫队友
						CallColor(g, targetX, targetY);
					}
					else {
						g.Wait();
						AttackCell(g, targetX, targetY);
					}
				}
			}//if (end_x >= start_x && end_y >= start_y)
			for (int i = 0; i < DIRECTION_NUM; ++i) {
				double forwardX = 200 * cos(i * angleUnit);
				double forwardY = 200 * sin(i * angleUnit);
				double nextPointX = self.x;
				double nextPointY = self.y;
				double multiplier = 1;
				for (int j = 0; j < 10; ++j) {
					nextPointX += forwardX;
					nextPointY += forwardY;
					int nextCellX = nextPointX / 1000;
					int nextCellY = nextPointY / 1000;
					if (nextCellX >= 0 && nextCellX < 50 && nextCellY >= 0 && nextCellY < 50 && isEmpty(nextCellX, nextCellY) && colorMap[nextCellX - cellX + 5][nextCellY - cellY + 5] == 0) {
						colorPoints[i] += multiplier;
					}
					multiplier *= COLOR_DECAY_RATE;
				}
			}//for (int i = 0; i < DIRECTION_NUM; ++i)
				
			if (holdingSchoolBag && grades3Conv[5][5] != 0) {
				g.Use();
			}
			else {
				if (self.propType == THUAI4::PropType::Null)g.Use();			//没有道具就先把手中的用了
			}

//发送常规消息
			SendNormalMSG(g);
//接收消息
			while (getMSGs(g) == MSGhead::CallColor) {
				//处理染色呼叫
				int attackX = MSG[3];
				int attackY = MSG[4];
				g.Wait();
				AttackCell(g, attackX, attackY);
			}
			while (getMSGs(g) == MSGhead::CallAttack) {
				//接收到攻击呼叫：仅在距离够近时发动攻击
				int callerID = MSGhead::LocID(MSG[0]);
				//处理队友的攻击呼叫
				u16 enemyX = (decodeu16DataHL(MSG[3], MSG[4]) << 2) + 2;
				u16 enemyY = (decodeu16DataHL(MSG[5], MSG[6]) << 2) + 2;
				u16 enemyHP = (decodeu16DataHL(MSG[7], MSG[8]) << 2) + 2;
				u16 enemySpeed = (decodeu16DataHL(MSG[9], MSG[10]) << 2) + 2;
				bool ismoving = (int)MSG[11];

				//与敌人的距离
				double distanceToEnemy = sqrt(pow((double)self.y - (double)enemyY, 2) + pow((double)self.x - (double)enemyX, 2));
				if (distanceToEnemy < 25000) {
					//打三发子弹，并向敌人靠近
					if (self.bulletNum > 4) {
						for (int i = 0; i < 5; ++i) {
							g.Wait();
							AttackPoint(g, enemyX, enemyY);
						}
					}
					else {
						//打完
						for (int i = 0; i < self.bulletNum; ++i) {
							g.Wait();
							AttackPoint(g, enemyX, enemyY);
						}
					}
				}
				//敌人的方向
				double enemyAngle = atan(((double)enemyY - (double)self.y) / ((double)enemyX - (double)self.x));
				if ((double)enemyX - (double)self.x < 0)enemyAngle += PI;
				else if ((double)enemyY - (double)self.y)enemyAngle += twoPI;

				movingAngle = enemyAngle;
				//进入探索模式
				exploreMode = 1;
				ExploreTime = 3000;
				TimeEnteringExploreMode = TIME_SINCE_START;
			}

//感知子弹
			int foundBullets = 0;
			double risks[DIRECTION_NUM];
			for (int i = 0; i < DIRECTION_NUM; ++i)risks[i] = 0;
			//评估各个方向风险
			auto visibleBullets = g.GetBullets();
			for (auto it = visibleBullets.begin(); it != visibleBullets.end(); ++it) {
				if ((*it)->bulletType == THUAI4::BulletType::Bullet1 || (*it)->bulletType == THUAI4::BulletType::Bullet4) {
					continue;			//忽略掉这两种子弹
				}
				if ((*it)->teamID != self.teamID) {
					foundBullets = 1;
					for (int i = 0; i < DIRECTION_NUM; ++i) {
						//PI以内的运动角度差
						double theta = (double)i * angleUnit - (*it)->facingDirection;
						if (theta > PI)theta -= twoPI;
						if (theta < -PI)theta += twoPI;

						//计算出子弹相对角色的运动方向
						double psi = (((double)self.moveSpeed) * sin(theta)) / (((double)((*it)->moveSpeed)) - ((double)self.moveSpeed) * cos(theta));
						psi = (*it)->facingDirection - psi;
						if (psi > PI)psi -= twoPI;
						if (psi < -PI)psi += twoPI;

						//计算出子弹相对角色的运动速度
						theta = PI - abs(theta);
						double relativeVel = (double)(((*it)->moveSpeed) * ((*it)->moveSpeed)) + (double)(self.moveSpeed * self.moveSpeed) - 2 * ((double)(((*it)->moveSpeed) * self.moveSpeed)) * cos(theta);
						relativeVel = sqrt(relativeVel);

						//计算出子弹在径向靠近（远离）和横向靠近（远离）的速度：以远离为正方向
						double objDir = atan((((double)((*it)->y)) - ((double)self.y)) / (((double)((*it)->x)) - ((double)self.x)));
						if (((double)((*it)->x)) - ((double)self.x) < 0)objDir += PI;
						else if (((double)((*it)->y)) - ((double)self.y))objDir += twoPI;

						double relativeDir = psi - objDir;
						if (relativeDir > PI)relativeDir -= twoPI;
						if (relativeDir < -PI)relativeDir += twoPI;

						double radialVel = relativeVel * cos(relativeDir);				//正方向为远离
						double normalVel = relativeDir * sin(relativeDir);				//正方向为逆时针
						double risk = 0;
						if (radialVel < 0) {
							//计算到子弹的距离
							double distanceToBullet = sqrt((((double)((*it)->y)) - ((double)self.y)) * (((double)((*it)->y)) - ((double)self.y)) + (((double)((*it)->x)) - ((double)self.x)) * (((double)((*it)->x)) - ((double)self.x)));
							//评估风险
							risk = -radialVel / distanceToBullet;
						}
						risks[i] += risk;
					}//for (int i = 0; i < DIRECTION_NUM; ++i)
				}
			}//for (auto it = visibleBullts.begin(); it != visibleBullets.end(); ++it)

			//寻找风险最小的可前进方向
			if (foundBullets) {
				int minRiskAvailableDir = 0;
				double minRisk = 10000000;
				for (int i = 0; i < DIRECTION_NUM; ++i) {
					if (risks[i] < minRisk && safeDistances[i] > CRITICAL_PASSABILITY) {
						minRiskAvailableDir = i;
						minRisk = risks[i];
					}
				}
				movingAngle = minRiskAvailableDir * angleUnit;
				g.Wait();
				Move(g);
				continue;
			}

//处理道具信息
			int foundProps = 0;
			auto visibleProps = g.GetProps();
			for (auto it = visibleProps.begin(); it != visibleProps.end(); ++it) {
				if (!isPickable((*it)->guid))continue;			//忽略拾取能力之外的道具
				if (!isEmpty((*it)->x / 1000, (*it)->y / 1000))continue;
				//if (isNeglected((*it)->guid))continue;			//跳过应该忽略的道具
				//if ((*it)->guid != targetPropGUID) {
				//	//先发消息
				//	timeDiscoveringTargetProp = TIME_SINCE_START;
				//	AbnormalMSG::DeclarePropOwnership(g, (*it)->guid, timeDiscoveringTargetProp);
				//	targetPropGUID = (*it)->guid;
				//}
				if ((*it)->x / 1000 == self.x / 1000 && (*it)->y / 1000 == self.y / 1000) {
					g.Wait();
					if (g.Pick((*it)->propType)) {
						//成功捡起了道具
						switch ((*it)->propType) {
							//地雷型道具就地埋藏（简化设计）
						case THUAI4::PropType::MusicPlayer:
						case THUAI4::PropType::Puddle:
						case THUAI4::PropType::Mail:
							g.Use();
							break;

						case THUAI4::PropType::Bike: {
							if (self.propType != THUAI4::PropType::Null) {
								//如果自己有道具，就优先扔给队友。队友的优先级
								for (u32 i = 0; i < PlayerNum; ++i) {
									if (i != myLocalID && teamMates[i].propType == 0 && teamMates[i].isDying == 0) {
										g.Wait();
										ThrowToCell(g, teamMates[i].x / 1000, teamMates[i].y / 1000);
									}
								}
							}//if (self.propType == THUAI4::PropType::Bike)
							else {
								//如果自己手中没有道具，就使用了
								g.Use();
							}
							break;
						}//case THUAI4::PropType::Bike

						case THUAI4::PropType::SchoolBag: {
							//如果是自己的第一个书包，先藏起来
							if (!havingPropAtBirthPoint) {
								ThrowToCell(g, myBirthPointX, myBirthPointY);
								havingPropAtBirthPoint = 1;
							}
							//如果自己身处可恢复子弹的区域，就直接使用，否则，等会儿再使用。
							if (grades3Conv[5][5] != 0)g.Use();
							else {
								holdingSchoolBag = 1;
							}
							break;
						}//case THUAI4::PropType::SchoolBag

						case THUAI4::PropType::Horn:
						case THUAI4::PropType::Javelin:
						case THUAI4::PropType::Clothes: 
						case THUAI4::PropType::Shield: {
							//优先给没有道具的人使用
							int userID = -1;
							for (int i = 0; i < PlayerNum; ++i) {
								if (teamMates[i].propType == 0 && teamMates[i].isDying == 0) {
									userID = i;
								}
							}
							if (userID != -1 && userID != myLocalID) {
								g.Wait();
								ThrowToCell(g, teamMates[userID].x / 1000, teamMates[userID].y / 1000);
							}
							break;									//没有用上就留着好了
						}
							
						case THUAI4::PropType::HappyHotPot: {
							//优先给HP最少的角色使用
							int minHP = 10000;
							int minHPID = 0;
							for (int i = 0; i < PlayerNum; ++i) {
								if (teamMates[i].hp < minHP && teamMates[i].isDying == 0) {
									minHP = teamMates[i].hp;
									minHPID = i;
								}
							}
							if (minHPID == myLocalID)g.Use();
							else {
								//扔给需要的人
								g.Wait();
								ThrowToCell(g, teamMates[minHPID].x / 1000, teamMates[minHPID].y / 1000);
							}
							break;
						}//case THUAI4::PropType::HappyHotPot

						default:
							g.Use();
							break;
						}//switch ((*it)->propType)
					}//if (g.Pick((*it)->propType))
				}
				else {
					foundProps = 1;
					//算法推荐的方向
					double recommendedDir = PlanPathToCell(self.x / 1000, self.y / 1000, (*it)->x / 1000, (*it)->y / 1000);

					if (recommendedDir == -1) {
						//不可拾取
						markAsUnpickable((*it)->guid);
						continue;
					}
					//直线方向
					double linearDir = atan(((double)(*it)->y - (double)self.y) / ((double)(*it)->x - (double)self.x));
					if (((int)(*it)->x - (int)self.x) < 0)linearDir += PI;
					else if (((int)(*it)->y - (int)self.y) < 0)linearDir += twoPI;
					int propDirInd = linearDir / angleUnit;
					//直线距离
					int linearDistance = (int)(sqrt(((double)(*it)->x - (double)self.x) * ((double)(*it)->x - (double)self.x) + ((double)(*it)->y - (double)self.y) * ((double)(*it)->y - (double)self.y)));
					if (linearDistance < 700) {
						moveDuration = (int)(1000.0 * (double)linearDistance / (double)self.moveSpeed);
					}
					linearDistance /= 1000;
					//计算出角度差
					double dirDiff = linearDir - recommendedDir;
					if (dirDiff > PI)dirDiff -= twoPI;
					if (dirDiff < -PI)dirDiff += twoPI;

					double coeficient = 1;
					for (int i = 0; i < linearDistance; ++i) {
						coeficient *= DIRECTION_DECAY_RATE;
					}

					recommendedDir += dirDiff * coeficient;
					if (recommendedDir < 0)recommendedDir += twoPI;
					if (recommendedDir >= twoPI)recommendedDir -= twoPI;

					//这是一个把角色引导向道具方向的角度，但是，决策也需要考虑可通过性：保证所前进方向的可通行距离不能小于一个阈值。
					int recommendDirInd = recommendedDir / angleUnit;

					//搜索可通过性：像两边寻找，看哪个方向最先遇到大于阈值的可通过性
					double found = 0;
					int clockwiseDir = recommendDirInd, counterClockwiseDir = recommendDirInd;
					int clockwiseCnt = 0, counterClockwiseCnt = 0;
					for (; clockwiseCnt <= DIRECTION_NUM / 2; ++clockwiseCnt) {
						//顺时针搜索
						int cur_dir = clockwiseCnt + recommendDirInd;
						if (cur_dir >= DIRECTION_NUM) {
							cur_dir -= DIRECTION_NUM;
						}
						if (safeDistances[cur_dir] > CRITICAL_PASSABILITY) {
							clockwiseDir = cur_dir;
							break;
						}
					}
					for (; counterClockwiseCnt <= DIRECTION_NUM / 2; ++counterClockwiseCnt) {
						//逆时针搜索
						int cur_dir = recommendDirInd - counterClockwiseCnt;
						if (cur_dir < 0) {
							cur_dir += DIRECTION_NUM;
						}
						if (safeDistances[cur_dir] > CRITICAL_PASSABILITY) {
							counterClockwiseDir = cur_dir;
							break;
						}
					}
					if (clockwiseCnt == DIRECTION_NUM / 2 && counterClockwiseCnt == DIRECTION_NUM / 2) {
						//表示没有找到，就直接搜索最大的
						double maxSafeD = 0;
						double maxSafeDisDir = 0;
						for (int i = 0; i < DIRECTION_NUM; ++i) {
							if (safeDistances[i] > maxSafeD) {
								maxSafeD = safeDistances[i];
								maxSafeDisDir = i;
							}
						}
						movingAngle = maxSafeDisDir * angleUnit;
					}
					else {
						if (clockwiseCnt > counterClockwiseCnt) {
							int ind = recommendDirInd - counterClockwiseCnt;
							if (ind < 0)ind += DIRECTION_NUM;
							movingAngle = ind * angleUnit;
						}
						else {
							int ind = recommendDirInd + clockwiseCnt;
							if (ind >= DIRECTION_NUM)ind -= DIRECTION_NUM;
							movingAngle = ind * angleUnit;
						}
					}
					//cout << "===GOING TOWARDS A PROP===\n";
				}
				break;
			}
			////如果没有道具
			//if (visibleProps.size() == 0) {
			//	timeDiscoveringTargetProp = TIME_SINCE_START;
			//	AbnormalMSG::DeclarePropOwnership(g, 0, timeDiscoveringTargetProp);
			//	targetPropGUID = 0;
			//}
			if (foundProps) { 
				g.Wait();
				Move(g);
				continue;
			}
			

			double plannedAngle = movingAngle;
			if (exploreMode) {
				//探索模式
				if (TIME_SINCE_START - TimeEnteringExploreMode > ExploreTime) {
					//本轮探索完成
					exploreMode = false;
				}
			}
			else {
				//非探索模式
				plannedAngle = PlanPathToExplore(self.x / 1000, self.y / 1000);
			}
			int plannedDirInd = plannedAngle / angleUnit;

			//搜索可通过性：像两边寻找，看哪个方向最先遇到大于阈值的可通过性
			int clockwiseDir = plannedDirInd, counterClockwiseDir = plannedDirInd;
			int clockwiseCnt = 0, counterClockwiseCnt = 0;
			for (; clockwiseCnt < DIRECTION_NUM / 2; ++clockwiseCnt) {
				//顺时针搜索
				int cur_dir = clockwiseCnt + plannedDirInd;
				if (cur_dir >= DIRECTION_NUM) {
					cur_dir -= DIRECTION_NUM;
				}
				if (passable[cur_dir] && safeDistanceConvs[cur_dir] > CRITICAL_PASSABILITY) {
					clockwiseDir = cur_dir;
					break;
				}
			}
			for (; counterClockwiseCnt < DIRECTION_NUM / 2; ++counterClockwiseCnt) {
				//逆时针搜索
				int cur_dir = plannedDirInd - counterClockwiseCnt;
				if (cur_dir < 0) {
					cur_dir += DIRECTION_NUM;
				}
				if (passable[cur_dir] && safeDistanceConvs[cur_dir] > CRITICAL_PASSABILITY) {
					counterClockwiseDir = cur_dir;
					break;
				}
			}
			if (clockwiseCnt > counterClockwiseCnt) {
				int ind = plannedDirInd - counterClockwiseCnt;
				if (ind < 0)ind += DIRECTION_NUM;
				movingAngle = ind * angleUnit;
			}
			else {
				int ind = plannedDirInd + clockwiseCnt;
				if (ind >= DIRECTION_NUM)ind -= DIRECTION_NUM;
				movingAngle = ind * angleUnit;
			}

			g.Wait();
			Move(g);
		}//while (true)
		break;
	}//case DEFAULT

	}//switch(mode)

}//play

void InitMap() {
	//初始化地图
	for (int i = 0; i < 50; ++i) {
		for (int j = 0; j < 50; ++j) {
			Map[i][j] = 0;
			MapValue[i][j] = 0;
			lastAttackTimes[i][j] = 0;
		}
	}
	//首先标明8个出生点
	Map[2][3] = 6;
	Map[3][2] = 6;
	Map[2][46] = 6;
	Map[3][47] = 6;
	Map[46][2] = 6;
	Map[47][3] = 6;
	Map[47][46] = 6;
	Map[46][47] = 6;
	//一些地方标注为墙壁，避免角色进入后卡死
	Map[21][1] = 1;
	Map[28][1] = 1;

	for (int i = 1; i <= 3; ++i) {
		Map[1][i] = 1;
		Map[48][i] = 1;
	}
	Map[2][1] = 1;
	Map[2][2] = 1;
	Map[3][1] = 1;
	Map[47][1] = 1;
	Map[47][2] = 1;
	Map[46][1] = 1;

	for (int i = 46; i <= 48; ++i) {
		Map[1][i] = 1;
		Map[48][i] = 1;
	}
	Map[2][47] = 1;
	Map[2][48] = 1;
	Map[3][48] = 1;
	Map[47][47] = 1;
	Map[47][48] = 1;
	Map[46][48] = 1;
}

//简化角色对象成员函数
SimplifiedCharacter::SimplifiedCharacter(THUAI4::Character& chara) :
	//这个构造函数仅仅用于创建自身的简化版copy
	isDying(chara.isDying), hp(encodeu16Data(chara.hp)), x(encodeu16Data(chara.x)), y(encodeu16Data(chara.y)),
	ap(encodeu16Data(chara.ap)), mvSpeed(encodeu16Data(chara.moveSpeed)),
	placeholder1(0), placeholder2(0), bulletNum(chara.bulletNum), jobType((u32)chara.jobType), propType((u32)chara.propType) {}
void SimplifiedCharacter::upDate(THUAI4::Character& chara) {
	//用于更新自身的信息
	isDying = chara.isDying;
	bulletNum = chara.bulletNum;
	propType = (u32)chara.propType;
	//需要解码的u16数据
	hp = encodeu16Data(chara.hp);
	x = encodeu16Data(chara.x);
	y = encodeu16Data(chara.y);
	ap = encodeu16Data(chara.ap);
	mvSpeed = encodeu16Data(chara.moveSpeed);
}
void SimplifiedCharacter::PrintChara() {
	cout << "Job type:\t" << (int)jobType
		<< "\nprop type:\t" << (int)propType << "\nx:\t\t" << x << "\ny:\t\t" << y << "\nhp:\t\t" << hp
		<< "\nbulletNum:\t" << (int)bulletNum << endl;
}

//消息处理函数
//
//角色信息
void NormalMSG::decodeChara(string& msg, SimplifiedCharacter& chara) {
	u8* ptr = (u8*)&chara;
	for (u32 cnt = MSGCntRec; cnt < CharaInfoLen + MSGCntRec; ++cnt, ++ptr) {
		*ptr = msg[cnt];
	}
	chara.x = decodeu16Data(chara.x);
	chara.y = decodeu16Data(chara.y);
	chara.hp = decodeu16Data(chara.hp);
	chara.ap = decodeu16Data(chara.ap);
	chara.mvSpeed = decodeu16Data(chara.mvSpeed);
	MSGCntRec += CharaInfoLen;
}
void NormalMSG::encodeChara(string& msg, SimplifiedCharacter& chara) {
	u8* ptr = (u8*)&chara;
	for (u32 cnt = MSGCntSend; cnt < MSGCntSend + CharaInfoLen; ++cnt, ++ptr) {
		msg[cnt] = *ptr;
	}
	MSGCntSend += CharaInfoLen;
}
//地图信息
void NormalMSG::decodeMapUpdate(string& msg) {
	//解析墙壁信息更新条目数
	int items = ((int)MSG[MSGCntRec]);
	++MSGCntRec;
	for (int i = 0; i < items; ++i) {

		//还原数据
		u16 data = MSG[MSGCntRec];
		++MSGCntRec;
		data += (MSG[MSGCntRec] << 7);
		++MSGCntRec;

		//解码出x, y_start, y_end
		int cellX = teamMates[MSGhead::LocID(msg[0])].x / 1000;
		int cellY = teamMates[MSGhead::LocID(msg[0])].y / 1000;
		int x = (data & (7 << 8)) >> 8;
		int y_start = ((data & (7 << 4)) >> 4);
		int y_end = (data & 7);
		if ((data & (1 << 11)))x = -x;
		if ((data & (1 << 7)))y_start = -y_start;
		if ((data & (1 << 3)))y_end = -y_end;
		x += cellX;
		y_start += cellY;
		y_end += cellY;

		//解析地图
		for (int cnt_y = y_start; cnt_y < y_end; ++cnt_y) {
			if (!isWall(x, cnt_y)) {
				MarkWall(x, cnt_y);
			}
		}
	}//for (int i = 0; i < items; ++i)
}
void NormalMSG::encodeMapUpdate(string& msg) {
	list<u16> updates = {};

	int cellX = self.x / 1000;
	int cellY = self.y / 1000;

	//四个方向的极限
	int LimitXD = max(cellX - radiusInCells, 0),
		LimitXU = min(cellX + radiusInCells, 49),
		LimitYD = max(cellY - radiusInCells, 0),
		LimitYU = min(cellY + radiusInCells, 49);
	for (int cnt_x = LimitXD; cnt_x <= LimitXU; ++cnt_x) {
		int cnt_y = LimitYD;														//计数的起点
		while (cnt_y != LimitYU) {
			//对这一行的每一段循环
			while ((!isWall(cnt_x, cnt_y)) && (cnt_y != LimitYU)) {			//跳过空白点
				++cnt_y;
			}
			int y_start = cnt_y - cellY;
			while ((isWall(cnt_x, cnt_y)) && (cnt_y != LimitYU)) {				//处理连续的一段墙壁
				++cnt_y;
			}
			int y_end = cnt_y - cellY;

			//对这一条记录进行编码
			int x = cnt_x - cellX;
			u16 data = 0;										//将要存储的数据
			//加入符号正负
			if (x < 0) {
				data += (1 << 11);
				x = -x;
			}
			if (y_start < 0) {
				data += (1 << 7);
				y_start = -y_start;
			}
			if (y_end < 0) {
				data += (1 << 3);
				y_end = -y_end;
			}
			//加入数据
			data += (x << 8);
			data += (y_start << 4);
			data += y_end;
			//编码
			updates.emplace_back(data);
		}
	}//for (int cnt_x = LimitXD; cnt_x <= LimitXU; ++cnt_x)

	//开始编码消息
	u8 updateLen = updates.size();
	msg[MSGCntSend] = updateLen;
	++MSGCntSend;
	for (auto it = updates.begin(); it != updates.end(); ++it) {
		//低位在前，高位在后
		msg[MSGCntSend] = encodeu16DataL(*it);
		++MSGCntSend;
		msg[MSGCntSend] = encodeu16DataH(*it);
		++MSGCntSend;
	}
}
//火力呼叫
#define ATTACK_CALL_MSG_LEN 17
void CallAttack(GameApi& g, const THUAI4::Character enemy) {
	string MSGtoSend(64, 0);
	//首先编码时间
	MSGtoSend[0] = MSGhead::MakeIDAbnormal(myLocalID, MSGhead::CallAttack);
	MSGhead::encodeTime(MSGtoSend);
	//编码坐标、血量、移动速度
	MSGtoSend[3] = encodeu16DataH((u16)enemy.x >> 2);
	MSGtoSend[4] = encodeu16DataL((u16)enemy.x >> 2);
	MSGtoSend[5] = encodeu16DataH((u16)enemy.y >> 2);
	MSGtoSend[6] = encodeu16DataL((u16)enemy.y >> 2);
	MSGtoSend[7] = encodeu16DataH((u16)enemy.hp >> 2);
	MSGtoSend[8] = encodeu16DataL((u16)enemy.hp >> 2);
	MSGtoSend[9] = encodeu16DataH((u16)enemy.moveSpeed >> 2);
	MSGtoSend[10] = encodeu16DataL((u16)enemy.moveSpeed >> 2);
	MSGtoSend[11] = (u8)enemy.isMoving;
	MSGCntSend = 12;
	//编码移动方向，第一步是转化为u64
	u64 data = 0;
	char* ptr1 = (char*)&(enemy.facingDirection);
	char* ptr2 = (char*)&data;
	for (int i = 0; i < 8; ++i, ++ptr1, ++ptr2) {
		*ptr2 = *ptr1;
	}
	encodeu64Data(MSGtoSend, data, MSGCntSend);
	//发送消息（当前，全场只有一个狙击手）
	for (u32 i = 0; i < PlayerNum; ++i) {
		if (i != myLocalID) {
			g.Send(i, MSGtoSend);
		}
	}
}
void CallColor(GameApi& g, int cellX, int cellY) {
	int maxBulletNum = 0;
	int targetID = -1;
	for (int i = 0; i < PlayerNum; ++i) {
		if (i != myLocalID && teamMates[i].bulletNum > 5) {
			targetID = i;
			maxBulletNum = teamMates[i].bulletNum;
		}
	}
	if (targetID != -1) {
		//呼叫攻击
		string MSGtoSend(64, 0);
		MSGtoSend[0] = MSGhead::MakeIDAbnormal(myLocalID, MSGhead::CallColor);
		MSGhead::encodeTime(MSGtoSend);

		MSGtoSend[3] = (u8)cellX;
		MSGtoSend[4] = (u8)cellY;

		g.Send(targetID, MSGtoSend);
	}
}
//时间相关
void MSGhead::encodeTime(string& msg) {
	u16 epochsSinceStart = EPOCHS_SINCE_START;
	msg[1] = (u8)(epochsSinceStart & 127);
	msg[2] = (u8)((epochsSinceStart & (127 << 7)) >> 7);
}
bool MSGhead::checkTime(string& msg, u32 epochDiff) {
	if (EPOCHS_SINCE_START - MSGhead::decodeTime(msg) <= epochDiff)return true;
	return false;
}
//初始化消息
void AbnormalMSG::SendInitMSG(GameApi& self, int IDto) {
	//发送初始化消息
	MSGCntSend = 3;
	u32 initMSGLen = MSGCntSend + CharaInfoLen;
	string MSGtoSend(initMSGLen, 0);
	MSGtoSend[0] = MSGhead::MakeIDAbnormal(myLocalID, MSGhead::Initialize);
	NormalMSG::encodeChara(MSGtoSend, teamMates[myLocalID]);
	self.Send(IDto, MSGtoSend);
}
void AbnormalMSG::SendExterialAttackInfo(GameApi& self, int cellX, int cellY) {
	string MSGtoSend(64, 0);
	MSGtoSend[0] = MSGhead::MakeIDAbnormal(myLocalID, MSGhead::ExteriorAttackInfo);
	MSGhead::encodeTime(MSGtoSend);

	MSGtoSend[3] = (u8)cellX;
	MSGtoSend[4] = (u8)cellY;

	for (int i = 0; i < PlayerNum; ++i) {
		if (i != myLocalID) {
			self.Send(i, MSGtoSend);
		}
	}

}
void AbnormalMSG::DeclarePropOwnership(GameApi& self, u64 guid, u64 timeOfDiscovery) {
	//准备消息
	string MSGtoSend(64, 0);
	MSGtoSend[0] = MSGhead::MakeIDAbnormal(myLocalID, MSGhead::UpdatePropAlloc);
	//此处不能用常规的方法编码时间，而是应该编码一个固定的数据
	MSGhead::encodeTime(MSGtoSend);

	//编码新的道具信息
	encodeGUID(MSGtoSend, guid, 3);

	//编码旧的道具信息
	encodeGUID(MSGtoSend, targetPropGUID, 13);

	//编码时间
	encodeGUID(MSGtoSend, timeDiscoveringTargetProp, 23);

	//发送
	for (int i = 0; i < PlayerNum; ++i) {
		if (i != myLocalID) {
			self.Send(i, MSGtoSend);
		}
	}
}
//视野外打击信息同步
//动作函数
//
//定点攻击
void AttackCell(GameApi& g, int CellX, int CellY) {
	AttackPoint(g, CellX * 1000 + 500, CellY * 1000 + 500);
}
void AttackPoint(GameApi& g, int X, int Y) {
	//计算目标位置到自己位置的差别
	double diffX = X - (int)self.x;
	double diffY = Y - (int)self.y;
	//计算子弹运动时间
	int distance = sqrt(pow(diffX, 2) + pow(diffY, 2));
	double timeInMiliseconds = distance * 1000 / Constants::BulletMoveSpeed::bullet1;
	//计算攻击角度
	double angle = atan(diffY / diffX);
	if (diffX < 0)angle += PI;
	else if (diffY < 0)angle += twoPI;
	//发动攻击
	g.Attack(timeInMiliseconds, angle);
	//int t = EPOCHS_SINCE_START;
	//while (EPOCHS_SINCE_START - t < 1);
}

void MoveToPoint(GameApi& g, int X, int Y) {
	double diffX = X - (double)self.x;
	double diffY = Y - (double)self.y;
	//计算运动时间
	double distance = sqrt(diffX * diffX + diffY * diffY);
	double timeInMiliseconds = distance * 1000 / Constants::Character::MoveSpeed::job1;
	//计算运动角度
	double angle = atan(diffY / diffX);
	if (diffX < 0)angle += PI;
	else if (diffY < 0)angle += twoPI;

	g.MovePlayer(timeInMiliseconds, angle);
}
void MoveToCell(GameApi& g, int cellX, int cellY) {
	MoveToPoint(g, cellX * 1000 + 500, cellY * 1000 + 500);
}
void Move(GameApi& g) {
	g.MovePlayer(moveDuration, movingAngle);
}
//扔道具
void ThrowToCell(GameApi& g, int cellX, int cellY) {
	//计算目标位置到自己位置的差别
	double diffX = cellX * 1000 + 500 - (int)self.x;
	double diffY = cellY * 1000 + 500 - (int)self.y;
	//计算子弹运动时间
	int distance = sqrt(pow(diffX, 2) + pow(diffY, 2));
	double timeInMiliseconds = distance * 1000 / Constants::Game::thrownPropMoveSpeed;
	//计算攻击角度
	double angle = atan(diffY / diffX);
	if (diffX < 0)angle += PI;
	else if (diffY < 0)angle += twoPI;
	//发动攻击
	g.Throw(timeInMiliseconds, angle);
}
//决策相关函数
//
//地图更新
void toExplore(u32 coordX, u32 coordY) {

	int cellX = coordX / 1000;
	int cellY = coordY / 1000;

	//四个方向的极限
	int LimitXD = max(cellX - radiusInCells, 0),
		LimitXU = min(cellX + radiusInCells, 49),
		LimitYD = max(cellY - radiusInCells, 0),
		LimitYU = min(cellY + radiusInCells, 49);
	//对每一行，找起始和终结的x，同时完成对编码。
	for (int cnt_x = LimitXD; cnt_x <= LimitXU; ++cnt_x) {
		//用于编码的数据
		int y_start = LimitYD;
		while (pow(y_start - cellY, 2) + pow(cnt_x - cellX, 2) > radiusInCellsSquared) {
			++y_start;
		}
		int y_end = LimitYU;
		while (pow(y_end - cellY, 2) + pow(cnt_x - cellX, 2) > radiusInCellsSquared) {
			--y_end;
		}
		for (int cnt_y = y_start; cnt_y <= y_end; ++cnt_y) {
			if (!isExplored(cnt_x, cnt_y)) {
				ExploreCell(cnt_x, cnt_y);
			}
		}
	}
}
void updateLocalMap(WALLS_PTR& visibleWalls) {
	//对墙壁进行标记
	for (int i = 0; i < visibleWalls.size(); ++i) {
		int cellX = visibleWalls[i]->x / 1000;
		int cellY = visibleWalls[i]->y / 1000;
		if (!isWall(cellX, cellY)) {
			MarkWall(cellX, cellY);
		}
	}
}
void printMap() {
	//用于debug
	for (int i = 0; i < 50; ++i) {
		for (int j = 0; j < 50; ++j) {
			if (!isExplored(i, j)) {
				printf("  ");
			}
			else {
				if (!isEmpty(i, j)) {
					printf("█");
				}
				else {
					printf("╬ ");
				}
			}

		}
		printf("\n");
	}
}
//消息收发
int getMSGs(GameApi& g) {
	//接收队友发来的消息。对于普通消息，直接进行信息的同步；对于特殊消息，返回特殊的值，然后在函数外进行处理。
	while (g.TryGetMessage(MSG)) {
		//确定队友身份
		u8 id = MSG[0];
		int IDfrom = MSGhead::LocID(id);
		//普通类型的消息
		if (MSGhead::NormalMode(id)) {
			MSGCntRec = 3;
			//人物信息更新
			if (MSGhead::UpdateCharaInfo(id)) {
				NormalMSG::decodeChara(MSG, teamMates[IDfrom]);
			}
			//地图更新
			if (MSGhead::UpdateMap(id)) {
				toExplore(teamMates[IDfrom].x, teamMates[IDfrom].y);
				NormalMSG::decodeMapUpdate(MSG);
			}
		}//if (MSGhead::NormalMode(id))

		//特殊类型的消息
		else {
			switch ((u8)MSGhead::AbnormalType(id)) {
			case MSGhead::Initialize: {
				NormalMSG::decodeChara(MSG, teamMates[IDfrom]);
				break;
			}//case MSGhead::Initialize
			case MSGhead::CallColor: {
				return MSGhead::CallColor;
				break;
			}
			case MSGhead::CallAttack: {
				return MSGhead::CallAttack;
				break;
			}//case MSGhead::CallAttack
			case MSGhead::ExteriorAttackInfo: {
				lastExplorationTime[(u32)MSG[3]][(u32)MSG[4]] = TIME_SINCE_START;
				break;
			}//case MSGhead::ExteriorAttackInfo
			//case MSGhead::UpdatePropAlloc: {
			//	u64 givenUpGUID = decodeGUID(MSG, 13);
			//	u64 declaredGUID = decodeGUID(MSG, 3);
			//	u64 declaredTime = decodeGUID(MSG, 23);
			//	cout << "received prop info:\t" << givenUpGUID << "\t" << declaredGUID << "\t" << declaredTime << endl;	
			//	if (givenUpGUID != 0) {
			//		removeFromNeglect(givenUpGUID);
			//	}
			//	if (declaredGUID != -1 && declaredGUID == targetPropGUID) {
			//		//队友的目标与自己的重了
			//		if (declaredTime < timeDiscoveringTargetProp) {
			//			//别人先发现，自己就放弃了吧
			//			targetPropGUID = -1;
			//			addToNeglect(declaredGUID);
			//		}
			//		else if (declaredTime == timeDiscoveringTargetProp) {
			//			//连时间都一样，就比ID大小：ID大的优先
			//			if (myLocalID < IDfrom) {
			//				targetPropGUID = -1;
			//				addToNeglect(declaredGUID);
			//			}
			//		}
			//	}
			//	break;
			//}//case MSGhead::UpdatePropAlloc
			case MSGhead::EnemyInfo: {
				//解码
				int receivedCellX = MSG[3];
				int receivedCellY = MSG[4];

				//先判断自己有没有添加这个敌人
				int exists = 0;
				for (auto it = enemyBirthPoints.begin(); it != enemyBirthPoints.end(); ++it) {
					if (it->cellX == receivedCellX && it->cellY == receivedCellY) {
						//找到了
						++(it->discoverCnt);
						exists = 1;
						break;
					}
				}
				if (exists == 0) {
					//没找到
					birthpoint newpoint(receivedCellX, receivedCellY, TIME_SINCE_START);
					enemyBirthPoints.emplace_back(newpoint);
				}
				break;
			}//case MSGhead::EnemyInfo
			default:break;
			}//switch (MSGhead::AbnormalType(id))
		}
	}//while (g.TryGetMessage(MSG))
	return 0;
}
void SendNormalMSG(GameApi& g) {
	//使用前请确保：地图已经更新，自身信息已经更新
	MSGCntSend = 3;
	string MSGtoSend(64, 0);
	//string MSGtoSend(MSGhead::MakeMSGLenNormal(true, true, false, false), 0);
	MSGtoSend[0] = MSGhead::MakeIDNormal(myLocalID, true, true, false, false);
	MSGhead::encodeTime(MSGtoSend);
	//编码角色
	teamMates[myLocalID].upDate(self);
	NormalMSG::encodeChara(MSGtoSend, teamMates[myLocalID]);
	//编码地图
	NormalMSG::encodeMapUpdate(MSGtoSend);
	//发送消息
	for (u32 cnt = 0; cnt < PlayerNum; ++cnt) {
		if (cnt != myLocalID) {
			g.Send(cnt, MSGtoSend);
		}
	}
}

//点到点的路径规划
#define PATH_PLAN_DEACY_RATE		0.5
double PlanPathToCell(int start_x, int start_y, int end_x, int end_y) {
	//输入数据：均为cell的坐标
	//规划特点：
	//最短距离；把未探索区域视作障碍物；如果终点无法到达，规划终止

	list<list<node>> model = {};
	list<node> l0 = {};
	node startPoint(start_x, start_y, 0, 0, NULL);
	node destination(end_x, end_y, 0, 0, NULL);
	l0.emplace_back(startPoint);
	model.emplace_back(l0);
	node endNode(0, 0, 0, 0, NULL);

	int seenDstination = false;
	int destinationReachable = true;
	for (u32 i = 0; i < 50; ++i) {
		for (u32 j = 0; j < 50; ++j) {
			UnexploreInSimu(i, j);					//将特殊位初始化
		}
	}
	ExploreInSimu(startPoint.x, startPoint.y);

	int layerCnt = 0;
	while (!seenDstination && destinationReachable) {															//Layer层次的循环
		//模型的最后一层
		list<node> nn = {};
		model.emplace_back(nn);
		auto curLayer = model.end();
		--curLayer;
		--curLayer;
		auto endLayer = model.end();
		--endLayer;
		destinationReachable = false;
		++layerCnt;

		//寻找下一个layer的点
		for (auto it = curLayer->begin(); it != curLayer->end(); ++it) {
			int x_coord = (it)->x, y_coord = (it)->y;
			int x_start = max(0, x_coord - 1);
			int x_end = min(49, x_coord + 1);
			int y_start = max(0, y_coord - 1);
			int y_end = min(49, y_coord + 1);

			//对八个方位进行循环
			for (u32 i = x_start; i <= x_end; ++i) {
				for (u32 j = y_start; j <= y_end; ++j) {
					if (i == x_coord && j == y_coord) continue;			//排除掉自身

					bool reachable = isExplored(i, j) && isEmpty(i, j);
					if (i != x_coord && j != y_coord) {
						reachable = reachable && isExplored(x_coord, j) && isEmpty(x_coord, j) && isExplored(i, y_coord) && isEmpty(i, y_coord);
					}
					if (reachable && !isExploredInSimu(i, j)) {		//判断是否是墙壁或者为以前层级中的点。如果不是，则需要探索该点

						double NewDistance = ((i == x_coord || j == y_coord) ? 1.0 : SQRT_2) + (it)->distance;
						if (!Exists(i, j)) {										//表示该节点还没有被添加过
							destinationReachable = true;
							//创建节点
							node newNode(i, j, NewDistance, 0, &(*it));
							endLayer->emplace_back(newNode);
							toExist(i, j);											//改为已创建
						}
						else {														//表示已经被添加过的节点
							//先把这个点找到（从后往前找）
							auto IT = endLayer->end();
							if (IT != endLayer->begin())--IT;
							while (IT != endLayer->begin() && ((IT)->x != i || (IT)->y != j)) --IT;	//因为无论如何都可以找到，所以这一步是安全的
							//比较距离，如果找到更好的source，就更换。
							if (NewDistance < (IT)->distance) {
								(IT)->distance = NewDistance;
								//delete (*IT)->source;								//清理废物，只保留一条路线
								(IT)->source = &(*it);
							}
						}

					}//if (!isWall(i, j) && isExploredInSimu(i, j))
				}//for (u32 j = y_start; j <= y_end; ++j)
			}//for (u32 i = x_start; i <= x_end; ++i)
		}//for (auto it = curLayer->begin(); it != curLayer->end(); ++it)

		//初始化特殊位：是否为下一层以添加节点
		for (auto it = endLayer->begin(); it != endLayer->end(); ++it) {
			unExist((it)->x, (it)->y);
			ExploreInSimu((it)->x, (it)->y);
		}

		//判断是否到达终点
		for (auto it = endLayer->begin(); it != endLayer->end(); ++it) {
			if (((it)->x == destination.x && (it)->y == destination.y)) {
				seenDstination = true;													//结束循环
				endNode = *(it);
				break;
			}
		}

	}//while (!seenDstination)
	double result;
	if (!destinationReachable)result = -1;			//表示目标不可到达
	else {										//找到destination之后的操作
		result = 0;
		//计算第一步移动的最佳角度：对规划的前五步进行综合
		node* path_ptr = &endNode;
		node* previous_node = path_ptr->source;
		list<double> direction_changes;
		while (previous_node != NULL) {
			//计算出该次位移的方向
			double dirChange = atan((double)(path_ptr->y - previous_node->y) / (double)(path_ptr->x - previous_node->x));
			if (path_ptr->x - previous_node->x < 0)dirChange += PI;
			else if (path_ptr->y - previous_node->y < 0)dirChange += twoPI;
			//放在前面，方便遍历
			direction_changes.emplace_front(dirChange);
			//前往下一个
			path_ptr = previous_node;
			previous_node = path_ptr->source;
		}

		double previousAngle = 0;
		for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
			//完成数据的交接
			double angleDiff = *it - previousAngle;
			previousAngle = *it;
			//处理下角度差：所要的“角度差”是指顺时针旋转的角度，可正可负，但范围限制为PI以内
			if (angleDiff > PI)angleDiff -= twoPI;
			if (angleDiff < -PI)angleDiff += twoPI;
			*it = angleDiff;
		}

		double multiplier = 1;
		for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
			result += (*it) * multiplier;
			multiplier *= PATH_PLAN_DEACY_RATE;
		}
		if (result < 0)result += twoPI;
		if (result >= twoPI)result -= twoPI;
	}//else
	return result;
}
double PlanPathToExplore(int start_x, int start_y) {

	//输入数据：当前位置的cell坐标，没有固定的终点；需要访问全局的MapValue数据
	//规划特点：
	//“单位距离的Value达到最大”；如果遇到终点，则终止规划

	list<list<node>> model = {};
	node startPoint(start_x, start_y, 0, 0, NULL);
	node endNode(0, 0, -1, -1000000, NULL);			//最终选定的终点
	list<node> endNodes;					//用于存储各条路线的终点
	//加入endNodes的条件：不可扩展、规划步数满
	list<node> l1 = {};
	l1.emplace_back(startPoint);
	model.emplace_back(l1);
	//三个可以使规划停止的条件
	int proceedable = true;					//是否将可探索范围探索尽		
	int reachedEnd = false;					//是否规划满最大步数

	for (u32 i = 0; i < 50; ++i) {
		for (u32 j = 0; j < 50; ++j) {
			UnexploreInSimu(i, j);			//将特殊位初始化
		}
	}
	ExploreInSimu(startPoint.x, startPoint.y);

	int layerCnt = 0;
	//Layer层次的循环
	while (proceedable && !reachedEnd) {
		//模型的最后一层
		list<node> nn = {};
		model.emplace_back(nn);
		auto curLayer = model.end();
		--curLayer;
		--curLayer;
		auto endLayer = model.end();
		--endLayer;

		proceedable = false;
		++layerCnt;

		//寻找下一个layer的点
		for (auto it = curLayer->begin(); it != curLayer->end(); ++it) {
			int x_coord = (it)->x, y_coord = (it)->y;
			int x_start = max(0, x_coord - 1);
			int x_end = min(49, x_coord + 1);
			int y_start = max(0, y_coord - 1);
			int y_end = min(49, y_coord + 1);
			//对八个方位进行循环
			int expandable = 0;					//这个节点是否可扩展
			for (u32 i = x_start; i <= x_end; ++i) {
				for (u32 j = y_start; j <= y_end; ++j) {

					if (i == x_coord && j == y_coord) continue;			//排除掉自身

					bool reachable = isEmpty(i, j);
					if (i != x_coord && j != y_coord) {
						reachable = reachable && isEmpty(x_coord, j) && isEmpty(i, y_coord);
					}
					if (reachable && !isExploredInSimu(i, j)) {						//判断是否是墙壁或者为以前层级中的点。如果不是，则需要探索该点

						proceedable = true;
						expandable = true;

						double NewDistance = ((i == x_coord || j == y_coord) ? 1.0 : SQRT_2) + (it)->distance;
						double newPoint = ((it)->points + lastExplorationTime[i][j]);
						if (!Exists(i, j)) {										//表示该节点还没有被添加过
							//创建节点
							node newNode(i, j, NewDistance, newPoint, &(*it));
							endLayer->emplace_back(newNode);
							toExist(i, j);											//改为已创建
						}
						else {														//表示已经被添加过的节点
							//先把这个点找到（从后往前找）
							auto IT = endLayer->end();
							if (IT != endLayer->begin())--IT;
							while (IT != endLayer->begin() && ((IT)->x != i || (IT)->y != j)) --IT;	//因为无论如何都可以找到，所以这一步是安全的
							//比较单位距离的得分，如果找到更好的source，就更换。
							if (newPoint / NewDistance < (IT)->points / (IT)->distance) {
								(IT)->distance = NewDistance;
								(IT)->points = newPoint;
								(IT)->source = &(*it);
							}
						}
					}//if (!isWall(i, j) && isExploredInSimu(i, j))
				}//for (u32 j = y_start; j <= y_end; ++j)
			}//for (u32 i = x_start; i <= x_end; ++i)

			if (!expandable) {
				//不可扩展的节点：加入endNodes
				endNodes.emplace_back(*it);
			}

		}//for (auto it = cur_layer.begin(); it != cur_layer.end(); ++it)

		//初始化特殊位：是否为下一层以添加节点
		for (auto it = endLayer->begin(); it != endLayer->end(); ++it) {
			unExist((it)->x, (it)->y);
			ExploreInSimu((it)->x, (it)->y);
		}

		//判断是否规划步数已满（避免重复）
		if (proceedable && layerCnt > PATH_PLAN_DEPTH) {
			reachedEnd = true;
			//加入endNodes
			for (auto it = endLayer->begin(); it != endLayer->end(); ++it) {
				endNodes.emplace_back(*it);
			}
			//找出endNode，把其他的垃圾清理掉
			double maxGrade = -2147483640;
			double bulletRatio = ((double)self.bulletNum) / ((double)self.maxBulletNum);		//决策的参考系数
			for (auto it = endNodes.begin(); it != endNodes.end(); ++it) {
				//cout << "candidate path:\t" << (int)(it->x) << "\t" << (int)(it->y) << "\t" << it->points << "\t" << it->distance << "\t" << (it->points) / (it->distance) << endl;
				//进行结果的计算
				double result = 0;
				//计算第一步移动的最佳角度：对规划的前五步进行综合
				node* path_ptr = &(*it);
				node* previous_node = path_ptr->source;
				list<double> direction_changes;
				while (previous_node != NULL) {
					//计算出该次位移的方向
					//cout << path_ptr->y - previous_node->y << "\t" << path_ptr->x - previous_node->x << endl;
					double dirChange = atan((double)(path_ptr->y - previous_node->y) / (double)(path_ptr->x - previous_node->x));
					if (path_ptr->x - previous_node->x < 0)dirChange += PI;
					else if (path_ptr->y - previous_node->y < 0)dirChange += twoPI;
					//放在前面，方便遍历
					direction_changes.emplace_front(dirChange);
					//前往下一个
					path_ptr = previous_node;
					//cout << "angle:\t" << dirChange << endl;
					previous_node = path_ptr->source;
				}

				double previousAngle = 0;
				for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
					//完成数据的交接
					double angleDiff = *it - previousAngle;
					previousAngle = *it;
					//处理下角度差：所要的“角度差”是指顺时针旋转的角度，可正可负，但范围限制为PI以内
					if (angleDiff > PI)angleDiff -= twoPI;
					if (angleDiff < -PI)angleDiff += twoPI;
					*it = angleDiff;
				}

				double multiplier = 1;
				for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
					result += (*it) * multiplier;
					multiplier *= 0.5;
				}
				if (result < 0)result += twoPI;
				if (result >= twoPI)result -= twoPI;
				int dir = result / angleUnit;
				double exploreValue = (it->points / it->distance);
				double grade = -(it->points / it->distance) + 5 * (6000 - 2 * teamScore) * exp(-10 * bulletRatio) * colorPoints[dir];

				if (grade > maxGrade) {
					maxGrade = grade;
					endNode = *it;
				}
			}
		}

		//判断是否无法推进（如果出现，则所有的终节点都已经加入endNodes）
		if (!proceedable) {
			//找出endNode，把其他的垃圾清理掉
			double maxGrade = -2147483640;
			double bulletRatio = ((double)self.bulletNum) / ((double)self.maxBulletNum);		//决策的参考系数
			for (auto it = endNodes.begin(); it != endNodes.end(); ++it) {
				//cout << "candidate path:\t" << (int)(it->x) << "\t" << (int)(it->y) << "\t" << it->points << "\t" << it->distance << "\t" << (it->points) / (it->distance) << endl;
				//进行结果的计算
				double result = 0;
				//计算第一步移动的最佳角度：对规划的前五步进行综合
				node* path_ptr = &(*it);
				node* previous_node = path_ptr->source;
				list<double> direction_changes;
				while (previous_node != NULL) {
					//计算出该次位移的方向
					//cout << path_ptr->y - previous_node->y << "\t" << path_ptr->x - previous_node->x << endl;
					double dirChange = atan((double)(path_ptr->y - previous_node->y) / (double)(path_ptr->x - previous_node->x));
					if (path_ptr->x - previous_node->x < 0)dirChange += PI;
					else if (path_ptr->y - previous_node->y < 0)dirChange += twoPI;
					//放在前面，方便遍历
					direction_changes.emplace_front(dirChange);
					//前往下一个
					path_ptr = previous_node;
					//cout << "angle:\t" << dirChange << endl;
					previous_node = path_ptr->source;
				}

				double previousAngle = 0;
				for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
					//完成数据的交接
					double angleDiff = *it - previousAngle;
					previousAngle = *it;
					//处理下角度差：所要的“角度差”是指顺时针旋转的角度，可正可负，但范围限制为PI以内
					if (angleDiff > PI)angleDiff -= twoPI;
					if (angleDiff < -PI)angleDiff += twoPI;
					*it = angleDiff;
				}

				double multiplier = 1;
				for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
					result += (*it) * multiplier;
					multiplier *= 0.5;
				}
				if (result < 0)result += twoPI;
				if (result >= twoPI)result -= twoPI;
				int dir = result / angleUnit;
				double exploreValue = (it->points / it->distance);

				double grade = -(it->points / it->distance) + 5 * (6000 - 2 * teamScore) * exp(-10 * bulletRatio) * colorPoints[dir];
				if (grade > maxGrade) {
					maxGrade = grade;
					endNode = *it;
				}
			}
		}

	}//while (!seenDstination)

	//进行结果的计算
	double result = 0;
	//计算第一步移动的最佳角度：对规划的前五步进行综合
	node* path_ptr = &endNode;
	node* previous_node = path_ptr->source;
	list<double> direction_changes;
	while (previous_node != NULL) {
		//计算出该次位移的方向
		double dirChange = atan((double)(path_ptr->y - previous_node->y) / (double)(path_ptr->x - previous_node->x));
		if (path_ptr->x - previous_node->x < 0)dirChange += PI;
		else if (path_ptr->y - previous_node->y < 0)dirChange += twoPI;
		//放在前面，方便遍历
		direction_changes.emplace_front(dirChange);
		//前往下一个
		path_ptr = previous_node;
		previous_node = path_ptr->source;
	}

	double previousAngle = 0;
	for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
		//完成数据的交接
		double angleDiff = *it - previousAngle;
		previousAngle = *it;
		//处理下角度差：所要的“角度差”是指顺时针旋转的角度，可正可负，但范围限制为PI以内
		if (angleDiff > PI)angleDiff -= twoPI;
		if (angleDiff < -PI)angleDiff += twoPI;
		*it = angleDiff;
	}

	double multiplier = 1;
	for (auto it = direction_changes.begin(); it != direction_changes.end(); ++it) {
		result += (*it) * multiplier;
		multiplier *= 0.5;
	}
	if (result < 0)result += twoPI;
	if (result >= twoPI)result -= twoPI;

	return result;
}
void AbnormalMSG::SendEnemyInfo(GameApi& self, int cellX, int cellY) {
	string MSGtoSend(64, 0);
	MSGtoSend[0] = MSGhead::MakeIDAbnormal(myLocalID, MSGhead::EnemyInfo);
	MSGhead::encodeTime(MSGtoSend);

	MSGtoSend[3] = cellX;
	MSGtoSend[4] = cellY;

	for (int i = 0; i < PlayerNum; ++i) {
		if (i != myLocalID) {
			self.Send(i, MSGtoSend);
		}
	}
}
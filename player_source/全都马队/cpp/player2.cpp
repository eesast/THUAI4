#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与Game的方法同时访问
extern const bool asynchronous = false;

#include <random>
#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <cstring>
#include <chrono>
#include <algorithm>
using namespace std;
using namespace THUAI4;

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job4; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

namespace MSG {

#define int long long
#define belongcell(belongcell__x) ((int)(belongcell__x/1000))
#define d_belongcell(belongcell__x) ((int)(belongcell__x/1000.00))
#define abs(abs__x) (((abs__x)>0)?(abs__x):(-abs__x))

	const int start_time = 1618993950538ll;
	const double pi = 3.1415926;
	const int Massage_compress_time = 256;
	const int Massage_compress_multiply[4] = { 1, 256, 256 * 256, 256 * 256 * 256 };
	queue<shared_ptr<const THUAI4::Wall>> walls_info;

	inline int getTime() {
		return (1ll * std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count()) - start_time;
	}

	struct enemy_returning_node {
		int x, y; //敌人的坐标
		int job; //敌人的职业
		int speed; //敌人速度的大小
		double facingDirection; //敌人速度的方向
		bool operator < (const enemy_returning_node& u) const { //重载运算符以便set自动排序
			if (x != u.x)
				return x < u.x;
			return y < u.y;
		}
	};

class Massage_Node {
	public:
		char str[64];
	private:
		void base_write(int first_pos, int length, int val); //进制转换后，将val写入[first_pos,first_pos+length-1]中
		void base_read(int first_pos, int length, int& val);//读取[first_pos,first_pos+length-1]的串，转化为十进制后写入val
		int base_read(int first_pos, int length); //读取[first_pos,first_pos+length-1]的串，转化为十进制后写入val
		void update_myinfo(shared_ptr<const THUAI4::Character> myinfo);
		void update_enemies(vector<shared_ptr<const THUAI4::Character>>* enemies, shared_ptr<const THUAI4::Character> myinfo);
		void update_time();
		void update_walls();
	public:
		void update_all(shared_ptr<const THUAI4::Character> myinfo
			, vector<shared_ptr<const THUAI4::Character>>* enemies); //更新出通讯代码！
		Massage_Node() {
			//str.clear();
			memset(str, 0, sizeof str);
		}
		void sendMassage(GameApi* api);
		void read_mate_job(int& mate_job); //需要准备返回值的储存位置
		int read_mate_job(); //直接返回队友职业
		void read_mate_location(int& matex, int& matey); //需要准备两个返回值的储存位置
		pair<int, int> read_mate_location(); //直接返回一个坐标
		void read_enemies(set<enemy_returning_node>* enemies); //准备一个set<enemy_returning_node>，然后连着读所有队友传来信息的enemies，以便去重。
		void read_walls(pair<int, int>* walls); //准备一个pair数组和组内四个可覆盖元素[a,a+1,a+2,a+3]，调用read_walls(walls+a)。
		void read_walls(vector<pair<int, int>>* walls); //准备一个pair组成的vector，新的墙将被丢在后面。
		bool getMassage(GameApi* api);
	};

	void Massage_Node::base_write(int first_pos, int length, int val) { //进制转换后，将val写入[first_pos,first_pos+length-1]中
	//	cerr << "b w " << first_pos << " " << length << " " << val << endl;
		for (int i = 0; i < length; i++) {
			str[first_pos + length - i - 1] = (char)((unsigned char)
				((val / Massage_compress_multiply[i]) % Massage_compress_time));
		}
	}

	void Massage_Node::base_read(int first_pos, int length, int& val) { //读取[first_pos,first_pos+length-1]的串，转化为十进制后写入val
		val = 0;
		for (int i = 0; i < length; i++)
			val += str[first_pos + length - i - 1] * Massage_compress_multiply[i];
	}

	int Massage_Node::base_read(int first_pos, int length) { //读取[first_pos,first_pos+length-1]的串，转化为十进制后写入val
	//	cerr << "b r " << first_pos << " " << length << endl;
		int val = 0;
		for (int i = 0; i < length; i++)
			val += str[first_pos + length - i - 1] * Massage_compress_multiply[i];
		return val;
	}

	void Massage_Node::update_myinfo(shared_ptr<const THUAI4::Character> myinfo) {
		base_write(0, 1, (int)((unsigned char)(myinfo->jobType)));
		base_write(1, 3, myinfo->x);
		base_write(4, 3, myinfo->y);
	}

	void Massage_Node::update_enemies(vector<shared_ptr<const THUAI4::Character>>* enemies, shared_ptr<const THUAI4::Character> myinfo) {
		//from 7 to 42
		vector<shared_ptr<const THUAI4::Character>>::iterator ic;
		int i = 0;
		for (ic = enemies->begin(); ic != enemies->end(); i++, ic++) {
			if ((*ic)->teamID == myinfo->teamID) {
				i--;
				continue;
			}
			base_write(9 * i + 7, 2, (*ic)->x - myinfo->x + 10000); //用笛卡尔坐标表示相对位置
			base_write(9 * i + 9, 2, (*ic)->y - myinfo->y + 10000);
			base_write(9 * i + 11, 1, (int)((unsigned char)((*ic)->jobType)));
			if ((*ic)->isMoving) {
				base_write(9 * i + 12, 2, (int)(((*ic)->facingDirection) / (2 * pi) * 180.0 * 180.0));
				base_write(9 * i + 14, 2, (*ic)->moveSpeed);
			}
			else {
				base_write(9 * i + 12, 2, 0);
				base_write(9 * i + 14, 2, 0);
			}
		}
		while (i < 4) {
			base_write(9 * i + 7, 2, 0);
			base_write(9 * i + 9, 2, 0);
			base_write(9 * i + 11, 1, 0);
			base_write(9 * i + 12, 2, 0);
			base_write(9 * i + 14, 2, 0);
			i++;
		}
	}
																				
	void Massage_Node::update_time() {
		//43~45
		base_write(43, 3, getTime());
	}

	void Massage_Node::update_walls() {
		//46~53
		for (int i = 0; i < 4 && (!walls_info.empty()); i++) {
			base_write(46 + i * 2, 1, belongcell(walls_info.front()->x));
			base_write(47 + i * 2, 1, belongcell(walls_info.front()->y));
			walls_info.pop();
		}
	}

	void Massage_Node::update_all(shared_ptr<const THUAI4::Character> myinfo
		, vector<shared_ptr<const THUAI4::Character>>* enemies) {
		update_myinfo(myinfo);
		update_enemies(enemies, myinfo);
		update_time();
		update_walls();
	}

	void Massage_Node::sendMassage(GameApi* api) {
		string strin = str;
		for (int i = 0; i < 4; i++)
			api->Send(i, strin);
	}

	void Massage_Node::read_mate_job(int& mate_job) {
		//需要准备返回值的储存位置
		mate_job = base_read(0, 1);
	}

	int Massage_Node::read_mate_job() { //直接返回队友职业
		return base_read(0, 1);
	}

	void Massage_Node::read_mate_location(int& matex, int& matey) {
		//需要准备两个返回值的储存位置
		matex = base_read(1, 3);
		matey = base_read(4, 3);
	}

	pair<int, int> Massage_Node::read_mate_location() { //直接返回一个坐标
		return pair<int, int>(base_read(1, 3), base_read(4, 3));
	}

	void Massage_Node::read_enemies(set<enemy_returning_node>* enemies) {
		//准备一个set<enemy_returning_node>，然后连着读所有队友传来信息的enemies，以便去重。
		enemy_returning_node tmp;
		int matex, matey;
		read_mate_location(matex, matey);
		for (int i = 0; i < 4; i++) { //更新包含的四个敌人
			tmp.x = base_read(9 * i + 7, 2) - 10000 + matex;
			tmp.y = base_read(9 * i + 9, 2) - 10000 + matey;
			tmp.job = base_read(9 * i + 11, 1);
			if (tmp.x == 0 && tmp.y == 0 && tmp.job == 0) //更新完了
				break;
			tmp.facingDirection = ((double)(base_read(9 * i + 12, 2))) * 2 * pi / 180.0 / 180.0;
			tmp.speed = base_read(9 * i + 14, 2);
			enemies->insert(tmp);
		}
	}

	void Massage_Node::read_walls(pair<int, int>* walls) {
		//准备一个pair数组和组内四个可覆盖元素[a,a+1,a+2,a+3]，调用read_walls(walls+a)。
		for (int i = 0; i < 4; i++) {
			walls[i].first = base_read(46 + i * 2, 1);
			walls[i].second = base_read(47 + i * 2, 1);
		}
	}

	void Massage_Node::read_walls(vector<pair<int, int>>* walls) {
		for (int i = 0; i < 4; i++) {
			walls->push_back(pair<int, int>(base_read(46 + i * 2, 1), base_read(47 + i * 2, 1)));
		}
	}

	bool Massage_Node::getMassage(GameApi* api) {
		if (!api->MessageAvailable())
			return false;
		string st;
		api->TryGetMessage(st);
		st.copy(str, 64, 0);
		//自动过滤掉自己发出去的信息！
		shared_ptr<const THUAI4::Character> myinfo = api->GetSelfInfo();
		if (base_read(0, 1) == ((int)((unsigned char)(myinfo->jobType)))) { //通过判断职业是否相同来过滤自己的信息
			if (!api->MessageAvailable())
				return false;
			api->TryGetMessage(st);
			st.copy(str, 64, 0);
		}
		//如果不希望过滤掉自己的信息，请删除这两条注释间的代码。
		return true;
	}

	inline bool cell_visible(int x, int y, shared_ptr<const THUAI4::Character> myinfo) {
		int divx = max(abs(((int)myinfo->x) - (x * 1000)), abs(((int)myinfo->x) - (x * 1000 + 999)));
		int divy = max(abs(((int)myinfo->y) - (y * 1000)), abs(((int)myinfo->y) - (x * 1000 + 999)));
		return (divx * divx) + (divy * divy) > 5000 * 5000;
	}

	struct enemy_node {
		shared_ptr<const THUAI4::Character> data;
		double val;
		enemy_node() {}
		enemy_node(shared_ptr<const THUAI4::Character> __data, double __val) : data(__data), val(__val) {}
		bool operator < (const enemy_node& u) const {
			return val < u.val;
		}
	};

}

using namespace MSG;

class Agent {
#define mp make_pair
#define xx first
#define yy second
#define cell(x) ((x) / 1000)
#define grid(x) ((x) * 1000 + 500)
#define inMap(x, y) ((x) >= 0 && (x) < 50 && (y) >= 0 && (y) < 50)
#define dis2(x1, y1, x2, y2) (((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2)))

	typedef THUAI4::ColorType Color;
	typedef THUAI4::Wall Wall;
	typedef pair<int, int> pii;

private:
	const int MOVE_SECOND = 50;
	const double PI = 3.1415926536;

private:
	Color mapCol[50][50];
	bool isWall[50][50];
	bool isBirth[50][50];
	bool isProp[50][50];
	bool visited[50][50];
	int dx[4] = { 1, -1, 0, 0 };
	int dy[4] = { 0, 0, 1, -1 };
	queue<pii> bQue;
	queue<pii> pQue;

private:
	GameApi& g;
	shared_ptr<const THUAI4::Character> self;
	Color selfColor;
	int selfX;
	int selfY;
	int selfCellX;
	int selfCellY;
	int selfScore;
	vector<shared_ptr<const THUAI4::Character>> vecCharacters;
	vector<shared_ptr<const THUAI4::Wall>> vecWalls;
	vector<shared_ptr<const THUAI4::Prop>> vecProps;
	vector<shared_ptr<const THUAI4::Bullet>> vecBulls;
	vector<shared_ptr<const THUAI4::BirthPoint>> vecBirthPoints = g.GetBirthPoints();
	set<enemy_returning_node> setGetEnemies;

public:
	Agent(GameApi& _g);
	int bfs(queue<pii>& q, int sx, int sy);
	void massagePart();
	void explorePart();
	void attack(int x, int y);
	void attackPart();
	void pickPart();
	void movePart();
};

int Agent::bfs(queue<pii>& q, int sx, int sy) {
	while (!q.empty()) {
		int x = q.front().xx;
		int y = q.front().yy;
		q.pop();
		for (int i = 0; i < 4; ++i) {
			int nx = x + dx[i];
			int ny = y + dy[i];
			if (!inMap(nx, ny))
				continue;
			if (isWall[nx][ny])
				continue;
			if (nx == sx && ny == sy)
				return i ^ 1;
			if (visited[nx][ny] == false) {
				visited[nx][ny] = true;
				q.push(mp(nx, ny));
			}
		}
	}
	return -1;
}
// initialize an agent with functions
Agent::Agent(GameApi& _g) :g(_g) {
	self = g.GetSelfInfo();
	selfX = self->x;
	selfY = self->y;
	selfCellX = cell(selfX);
	selfCellY = cell(selfY);
	selfColor = g.GetSelfTeamColor();
	selfScore = g.GetTeamScore();

	vecCharacters = g.GetCharacters();
	vecWalls = g.GetWalls();
	vecProps = g.GetProps();
	vecBulls = g.GetBullets();
	vecBirthPoints = g.GetBirthPoints();

	for (int i = 0; i < 50; ++i)
		for (int j = 0; j < 50; ++j) {
			mapCol[i][j] = Color::Invisible;
			isWall[i][j] = false;
			isProp[i][j] = false;
			isBirth[i][j] = false;
			visited[i][j] = false;
		}
}
// explore Color, Wall, Prop, BirthPoint
void Agent::massagePart() {
	Massage_Node* sedmes = new Massage_Node();
	sedmes->update_all(self, &vecCharacters);
	Massage_Node* getmes = new Massage_Node();
	getmes->getMassage(&g);
	pii vecGetWalls[4];
	getmes->read_walls(vecGetWalls);
	for (int i = 0; i < 4; ++i) {
		int x = vecGetWalls[i].xx;
		int y = vecGetWalls[i].yy;
		if (inMap(x, y))
			isWall[x][y] = true;
	}
	getmes->read_enemies(&setGetEnemies);
}
void Agent::explorePart() {
	while (!bQue.empty())
		bQue.pop();
	memset(visited, 0, sizeof visited);
	for (int i = 0; i < 50; ++i)
		for (int j = 0; j < 50; ++j) {
			if (dis2(grid(i), grid(j), selfX, selfY) > 25000000)
				continue;
			auto col = g.GetCellColor(i, j);
			if (col != Color::Invisible) {
				mapCol[i][j] = col;
				isProp[i][j] = false;
			}
			if (mapCol[i][j] == selfColor) {
				bQue.push(mp(i, j));
				visited[i][j] = true;
			}
		}

	for (auto pBirth : vecBirthPoints) {
		int x = cell(pBirth->x);
		int y = cell(pBirth->y);
		if (pBirth->teamID != self->teamID) {
			isWall[x][y] = true;
		}
		isBirth[x][y] = true;
	}
	for (auto pWall : vecWalls) {
		isWall[cell(pWall->x)][cell(pWall->y)] = true;
	}
	for (auto pProp : vecProps) {
		isProp[cell(pProp->x)][cell(pProp->y)] = true;
	}
}
// attack at (x,y)
void Agent::attack(int x, int y) {
	double angle = atan2((double)y - selfY, (double)x - selfX);
	if (angle < 0.0)
		angle += 2 * PI;
	double dis = sqrt(dis2(x, y, selfX, selfY));
	g.Attack(dis / 24.0, angle);
}

void Agent::attackPart() {
	if (self->bulletNum == 0)
		return;
	int flag = 0;
	for (auto v : vecCharacters) {
		if (v->teamID != self->teamID) {
			attack(v->x, v->y);
			flag = 1;
			break;
		}
	}

	if (!flag && !setGetEnemies.empty()) {
		vector<pii> vecEnemy;
		for (auto v : setGetEnemies) {
			vecEnemy.push_back(mp(v.x, v.y));
		}
		int X = selfX;
		int Y = selfY;
		sort(vecEnemy.begin(), vecEnemy.end(),
			[X, Y](auto a, auto b) {
				int da = dis2(X, Y, a.xx, a.yy);
				int db = dis2(X, Y, b.xx, b.yy);
				return da < db;
			});
		int x = vecEnemy[0].xx, y = vecEnemy[0].yy;
		int CX = x / 1000;
		int CY = y / 1000;
		double angle = atan2((double)y - Y, (double)x - X);
		if (angle < 0.0)
			angle += 2 * PI;
		double dis = sqrt(dis2(X, Y, x, y));
		g.Attack(dis / 24.0, angle);
		flag = 1;
	}
}

void Agent::pickPart() {
	for (auto a = THUAI4::PropType::Bike; a <= THUAI4::PropType::Divider; a = (THUAI4::PropType)((int)a + 1)) {
		g.Pick(a);
		g.Use();
	}
}

void Agent::movePart() {
	if (self->bulletNum == 0) {
		if (mapCol[selfCellX][selfCellY] != selfColor) {
			//find self color
			int dir = bfs(bQue, selfCellX, selfCellY);
			switch (dir) {
			case 0:
				//g.MoveDown(MOVE_SECOND);
				g.MovePlayer(MOVE_SECOND, 0.0);
				break;
			case 1:
				//g.MoveUp(MOVE_SECOND);
				g.MovePlayer(MOVE_SECOND, PI);
				break;
			case 2:
				//g.MoveRight(MOVE_SECOND);
				g.MovePlayer(MOVE_SECOND, PI / 2.0);
				break;
			case 3:
				//g.MoveLeft(MOVE_SECOND);
				g.MovePlayer(MOVE_SECOND, 3.0 * PI / 2.0);
				break;
			default:
				g.MovePlayer(MOVE_SECOND * 10, direction(e));
				break;
			}
		}
		else {
			g.MovePlayer(MOVE_SECOND / 10, direction(e));
		}
	}
	else if (self->bulletNum > 2) {
	//find props
		while (!pQue.empty())
			pQue.pop();
		memset(visited, 0, sizeof visited);
		for (int x = 0; x < 50; ++x)
			for (int y = 0; y < 50; ++y)
				if (isProp[x][y]) {
					visited[x][y] = true;
					pQue.push(mp(x, y));
				}
		int dir = bfs(pQue, selfCellX, selfCellY);
		switch (dir) {
		case 0:
			//g.MoveDown(MOVE_SECOND);
			g.MovePlayer(MOVE_SECOND, 0.0);
			break;
		case 1:
			//g.MoveUp(MOVE_SECOND);
			g.MovePlayer(MOVE_SECOND, PI);
			break;
		case 2:
			//g.MoveRight(MOVE_SECOND);
			g.MovePlayer(MOVE_SECOND, PI / 2.0);
			break;
		case 3:
			//g.MoveLeft(MOVE_SECOND);
			g.MovePlayer(MOVE_SECOND, 3.0 * PI / 2.0);
			break;
		default:
			g.MovePlayer(MOVE_SECOND * 10, direction(e));
			break;
		}
	}
	else {
		g.MovePlayer(MOVE_SECOND * 10, direction(e));
	}
}

void AI::play(GameApi& g) {
	Agent* ag = new Agent(g);
	cerr << "call massage part" << endl;
	ag->massagePart();
	cerr << "call explore part" << endl;
	ag->explorePart();
	cerr << "call attack part" << endl;
	ag->attackPart();
	cerr << "call pick part" << endl;
	ag->pickPart();
	cerr << "call move part" << endl;
	ag->movePart();
	cerr << "call end part" << endl;
}
 
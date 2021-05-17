#include "AI.h"
#include "Constants.h"
using namespace std;
//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = false;
typedef long long ll;
#include<cmath>
#include<iostream>
#include <random>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <utility>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <set>
#include <sstream>
#include <cstring>
#include <string.h>

#include<cassert>
#define Assert Myassert
void Myassert(bool f)
{
	if (f == false)
	{
		cout << "Assertion failed!" << endl;
	}
	//	exit(1);
}


extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业
//const int TEAM_JOBS[4] = { 1, 1, 1, 1 };
int PLAYER_ID, TEAM_ID;
int TEAM_NUM;
int PLAYER_NUM = 4;
const bool NO_DEBUG_OUT = true;

const double pi = acos(-1);
const bool FIXED_MAP = true;
const bool USE_CYCLE_SHAPE = true; //是否使用圆形判断碰撞 强烈建议不使用，不然有概率卡墙
const int MOVE_DEFAULT_TIME_BLOCK = 180;
const int DEFAULT_IDLE_TIME = 12;
const int IDLE_TIME_AFTER_ACTION = 40;
const int SIGHT_RADIUS = 5000;

const int EXTRA_TIME_FOR_MOVE = 3;  //移动时间向下取整后增加的移动时间毫秒数，用于防卡墙
const double FANGKA_THRESHOLD = 555; //移动长度小于这个时将会启用防卡墙机制
const int FANGKA_MINIMUM_TIME = 10; //最少移动时间毫秒数，用于防卡墙
const double FANGKA_MINIMUM_ANGLE = 2 * pi / 16; //最少移动角度（弧度），用于防卡墙
const double MOVE_RAND_LENGTH = 10; //额外随机方向移动点到自己的距离 用于防卡墙
const bool FANGKA_EXTRA_POINT = false; //是否在欧几里得寻路模块中加一个关键点以防止卡墙

const bool TIME_TO_CEIL = false;    //时间是否是50ms的倍数
const double eps = 1e-7;


int BULLET_VELOCITY;
int BULLET_PAINT_R;



const double DOWN = 0;
const double RIGHT = pi / 2;
const double UP = pi;
const double LEFT = pi / 2 * 3;
const int CELL_N = 50;
const int GRID_N = 1000;
const int PIXEL_N = CELL_N * GRID_N;
const int PLAY_TIME_WINDOW = 50;	//asynchronous=false时一次play调用满的时间


namespace GEO
{
	//geometry start
	const double pi = acos(-1);
	bool is0(double x) { return x > -eps && x < eps; }
	struct point
	{
		double x, y;
		point(double _x = 0, double _y = 0) { x = _x;y = _y; }
		point operator+(const point& t)const { return point(x + t.x, y + t.y); }
		point operator-(const point& t)const { return point(x - t.x, y - t.y); }
		point operator*(const point& t)const { return point(x * t.x - y * t.y, x * t.y + y * t.x); }
		point operator*(const double& t)const { return point(x * t, y * t); }
		point operator/(const double& t)const { return point(x / t, y / t); }
		point operator-()const { return point(-x, -y); }
		double len()const { return sqrt(x * x + y * y); }
		double sqrl()const { return x * x + y * y; }
		point operator/(const point& t)const { return point(x * t.x + y * t.y, -x * t.y + y * t.x) / t.len() / t.len(); }
		double det(const point& t)const { return x * t.y - y * t.x; }
		double dot(const point& t)const { return x * t.x + y * t.y; }
		double len2()const { return x * x + y * y; }
		point unit()const { return (*this) / len(); }
		bool operator< (const point& t)const { return is0(x - t.x) ? (is0(y - t.y) ? 0 : y < t.y) : x < t.x; }
		bool operator==(const point& t)const { return is0(x - t.x) && is0(y - t.y); }
		bool operator!=(const point& t)const { return !is0(x - t.x) || !is0(y - t.y); }
		void out()const { cerr << x << "," << y << endl; }
		double rad()const { return atan2(y, x); }
		point conj()const { return point(x, -y); }
	};
	point unit_vector(double dir) { return point(cos(dir), sin(dir)); }
	struct line
	{
		bool is_segment; //[p, q]
		point p, q;
		line(point _p = point(0, 0), point _q = point(0, 0), bool _is_segment = true) {
			p = _p;
			q = _q;
			is_segment = _is_segment;
		}
		line(double px, double py, double qx, double qy, bool _is_segment = true) {
			p = point(px, py);
			q = point(qx, qy);
			is_segment = _is_segment;
		}
		point vec() const { return q - p; }
		bool operator == (const line& t) const { return p == t.p && q == t.q; }
		bool has(const point& x) const {
			if (!is0(vec().det(x - p) / (x - p).len())) return false;
			if (is_segment)
			{
				return (p - x).dot(q - x) < eps;
			}
			else return true;
		}
	};
	point LineIntersection(const line& a, const line& b)
	{
		//double t = -(a.p - b.p).det(vec(b)) / vec(a).det(vec(b));
		double t = (b.p - a.p).det(b.q - a.p) / (a.q - a.p).det(b.q - b.p);
		return a.p + (a.q - a.p) * t;
	}
}

const int NUM_OF_PROP_TYPE = 10;
THUAI4::PropType PropTypes[NUM_OF_PROP_TYPE + 1] = { THUAI4::PropType::Null, THUAI4::PropType::SharedBike, THUAI4::PropType::Horn, THUAI4::PropType::SchoolBag, THUAI4::PropType::HappyHotPot, THUAI4::PropType::Shield, THUAI4::PropType::Clothes, THUAI4::PropType::Javelin, THUAI4::PropType::Puddle, THUAI4::PropType::MusicPlayer, THUAI4::PropType::Mail };
const int PropValue[NUM_OF_PROP_TYPE + 1] = { 0, 1000, 1000, 100000, 1000, 1000, 1000, 1000, 1000, 1000, 1000 };
map<THUAI4::PropType, int> PropToId;
int GetPropId(THUAI4::PropType p) { if (PropToId.size() == 0) for (int i = 0; i <= NUM_OF_PROP_TYPE; i++) PropToId[PropTypes[i]] = i; return PropToId[p]; }

const int NUM_OF_JOB_TYPE = 7;
THUAI4::JobType JobTypes[NUM_OF_JOB_TYPE] = { THUAI4::JobType::Job0, THUAI4::JobType::Job1, THUAI4::JobType::Job2, THUAI4::JobType::Job3, THUAI4::JobType::Job4, THUAI4::JobType::Job5, THUAI4::JobType::Job6 };
map<THUAI4::JobType, int> JobToId;
int GetJobId(THUAI4::JobType p) { if (JobToId.size() == 0) for (int i = 0; i < NUM_OF_JOB_TYPE; i++) JobToId[JobTypes[i]] = i; return JobToId[p]; }




GameApi* G;
bool time_firstblood = true;
ll time_start, real_time_start;
int GetTime()
{
	auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (time_firstblood) real_time_start = time_start = msec, time_firstblood = false;
	return msec - time_start;
}
bool InMap(int cell_x, int cell_y)
{
	return cell_x >= 0 && cell_x < CELL_N&& cell_y >= 0 && cell_y < CELL_N;
}
inline ll GetDist2(ll a, ll b, ll c, ll d) { return (a - c) * (a - c) + (b - d) * (b - d); }
inline int ToCell(double x) { return int(x / GRID_N + eps); }
inline pair<int, int> ToCell(double x, double y) { return make_pair(ToCell(x), ToCell(y)); }
inline pair<int, int> ToCell(GEO::point p) { return make_pair(ToCell(p.x), ToCell(p.y)); }
GEO::point CellPos(int x, int y) { return GEO::point((x + 0.5) * GRID_N, (y + 0.5) * GRID_N); }

//Get adjacent cells with respect to grid (x,y) within euclidean distance r grids
vector<pair<int, int> > AdjCellsToGrid(int x, int y, int r)
{
	vector<pair<int, int> > ret;
	int xl = max(0, ToCell(x - r));
	int xr = min(CELL_N - 1, ToCell(x + r));
	int yl = max(0, ToCell(y - r));
	int yr = min(CELL_N - 1, ToCell(y + r));
	for (int px = xl; px <= xr; px++) for (int py = yl; py <= yr; py++)
	{
		bool intersect = false;
		for (int i = px; i <= px + 1; i++)
		{
			for (int j = py; j <= py + 1; j++)
			{
				if (GetDist2(i * GRID_N, j * GRID_N, x, y) <= 1ll * r * r)
				{
					intersect = true;
				}
			}
		}
		if (intersect) ret.push_back(make_pair(px, py));
	}
	return ret;
}

map<ll, pair<int, int> > guid_to_playerid;
map<ll, pair<int, int> > guid_to_spawn;

const string MAP_DATA =
R"(XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
X................................................X
X..B..........................................B..X
X.B...................XXXX.....................B.X
X................................................X
X................................................X
X..........X.....................................X
X..........X.............XXXX....................X
X..........X.....................................X
X..........X.....................................X
X..........X.....................................X
X..........X.....................................X
X..........X.....................................X
X..........X.....................................X
X..........X........................XXXXXXXXXXX..X
X..........X.....................................X
X..........X.....................................X
X..........X................XXXXXX...............X
X..........X................X....................X
X...........................X....................X
X...........................X....................X
X.XXXXXXXX..................X....................X
X.XXXXXXXX..................X.........XX.........X
X.XXXXXXXX............................XX.........X
X.XXXXXXXX............................XX.........X
X.XXXXXXXX............................XX.........X
X.XXXXXXXX............................XX.........X
X.XXXXXXXX..........X.................XX.........X
X.XXXXXXXX..........X............................X
X...................X............................X
X...................X............................X
X...................X............................X
X..............XXXXXX............................X
X................................................X
X................................................X
X...................................XXXXXXXXXXX..X
X................................................X
X................................................X
X................................................X
X................................................X
X................................................X
X.......................XXXX.....................X
X................................................X
X................................................X
X................................................X
X..........................XXXX..................X
X.B............................................B.X
X..B..........................................B..X
X................................................X
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX)";

bool info_debug = false;

void send_message(string s, int to = -1)
{
	Assert(s.size() <= 64);
	for (int i = 0; i < PLAYER_NUM; i++) if (i != PLAYER_ID && (i == to || to == -1)) G->Send(i, s);
}
class InfoCollector
{
#define UNKNOWN -1
#define NONE 0
#define COLOR1 1
#define COLOR2 2
#define COLOR3 3
#define COLOR4 4
#define WALL 5
#define BIRTH 6
#define SELFBIRTH 7
private:
	int getColorId(THUAI4::ColorType t)
	{
		if (t == THUAI4::ColorType::None) return NONE;
		if (t == THUAI4::ColorType::Color1) return COLOR1;
		if (t == THUAI4::ColorType::Color2) return COLOR2;
		if (t == THUAI4::ColorType::Color3) return COLOR3;
		if (t == THUAI4::ColorType::Color4) return COLOR4;
		Assert(false);
	}
	bool can_paint(int t) { return t != WALL && t != BIRTH && t != SELFBIRTH; }
	bool can_move(int t) { return t != WALL && t != BIRTH; }
public:

	bool parse(string& s)
	{
		stringstream ss(s);
		string type;
		ss >> type;
		if (type == "M")
		{
			string ch;
			int x, y;
			ss >> x >> y >> ch;
			Assert(ch.size() == 49);
			for (int i = 0; i < 7; i++)
			{
				for (int j = 0; j < 7; j++)
				{
					char c = ch[i * 7 + j];
					if (c >= '0' && c <= '9')
					{
						int t = c - '0';
						if (t == SELFBIRTH) t = BIRTH;
						update_cell(x + i, y + j, t);
					}
				}
			}
		}
		else if (type == "C")
		{
			int xl, yl, xr, yr, ti, t;
			ss >> xl >> yl >> xr >> yr >> t >> ti;
			if (t == SELFBIRTH) t = BIRTH;
			for (int x = xl; x <= xr; x++) for (int y = yl; y <= yr; y++) update_cell(x, y, t, ti);
		}
		else if (type == "P")
		{
			int i, x, y;
			ss >> i >> x >> y;
			team_pos[i] = make_pair(x, y);
		}
		else if (type == "E")
		{
			int guid, job, hp, bulletNum, x, y, speed, ti;
			double dir, pdir;
			ss >> guid >> job >> hp >> bulletNum >> x >> y >> dir >> speed >> pdir >> ti;
			enemy[guid].update(guid, job, hp, bulletNum, x, y, dir, speed, pdir, ti);
		}
		else if (type == "EH")
		{
			int guid, hp, ti;
			ss >> guid >> hp >> ti;
			enemy[guid].update_hp(hp, ti, false);
		}
		else if (type == "F")
		{
			return true;
		}
		else if (type == "CP")
		{
			GEO::point p;
			double angle;
			int ti;
			ss >> p.x >> p.y >> angle >> ti;
			update_purplefish_cells(p, angle, ti);
		}
		return false;
	}
	void CheckInbox()
	{
		int cnt = 1000;
		while (cnt > 0 && G->MessageAvailable())
		{
			cnt--;
			string s;
			if (G->TryGetMessage(s)) parse(s);
		}
	}
	void send_map(int ti)
	{
		for (int i = -1; i <= 0; i++) for (int j = -1; j <= 0; j++)
		{
			stringstream ss;
			int px = self_cx + i * 7;
			int py = self_cy + j * 7;
			ss << "M " << px << " " << py << " ";
			for (int tx = 0; tx < 7; tx++) for (int ty = 0; ty < 7; ty++)
			{
				int x = px + tx, y = py + ty;
				if (!InMap(x, y) || cell_time[x][y] < ti) ss << '?';
				else ss << cell[x][y];
			}
			send_message(ss.str());
		}
	}
	void send_position()
	{
		stringstream ss;
		ss << "P " << PLAYER_ID << " " << self_cx << " " << self_cy;
		send_message(ss.str());
	}


	InfoCollector()
	{
		memset(cell, -1, sizeof(cell));
		memset(cell_time, -1, sizeof(cell_time));
		memset(birth_teamid, -1, sizeof(birth_teamid));
		self_color = -1;
		for (int i = 0; i < CELL_N; i++)
		{
			update_cell(0, i, WALL, 1e9);
			update_cell(i, 0, WALL, 1e9);
			update_cell(CELL_N - 1, i, WALL, 1e9);
			update_cell(i, CELL_N - 1, WALL, 1e9);
		}
		if (FIXED_MAP)
		{
			memset(cell, 0, sizeof(cell));
			for (int i = 0; i < CELL_N; i++)
			{
				for (int j = 0; j < CELL_N; j++)
				{
					char c = MAP_DATA[i * (CELL_N + 1) + j];
					if (c == 'X') update_cell(i, j, WALL, 1e9);
					else if (c == 'B') update_cell(i, j, BIRTH, 1e9);
					else cell_time[i][j] = -1e5;
				}
			}
		}
	}

	THUAI4::Character self;
	int self_color = -1, enemy_color, score;
	GEO::point self_p;
	int self_cx, self_cy, self_birth_cx, self_birth_cy;
	int shield_end_time;
	int cell[CELL_N + 2][CELL_N + 2], cell_time[CELL_N + 2][CELL_N + 2], cell_paint_time[CELL_N][CELL_N];
	int birth_teamid[CELL_N + 2][CELL_N + 2];
	pair<int, int> team_pos[4];

	vector<THUAI4::Character> adj_char;
	vector<THUAI4::Prop> adj_prop;
	vector<THUAI4::Bullet> adj_bullet;

	int dism[CELL_N + 2][CELL_N + 2]; //Manhattan distance
	double prop_value[CELL_N][CELL_N];
	struct enemy_info
	{
		int upd_time, hp_time, last_death_time;
		ll guid;
		int job, hp, bulletNum, x, y, speed;
		double dir, pdir;
		enemy_info()
		{
			upd_time = hp_time = -1;
			last_death_time = 0;
		}
		void update_hp(int _hp, int ti, bool send)
		{
			if (_hp < hp || ti > hp_time)
			{
				hp = _hp;
				hp_time = ti;
				if (hp <= 0) last_death_time = ti;
				if (send)
				{
					stringstream ss;
					ss << "EH " << guid << " " << hp << " " << ti << endl;
					send_message(ss.str());
				}
			}
		}
		void update(ll _guid, int _job, int _hp, int _bulletNum, int _x, int _y, double _dir, int _speed, double _pdir, int ti)
		{
			update_hp(_hp, ti, false);
			if (ti < upd_time) return;
			upd_time = ti;
			guid = _guid; job = _job;  bulletNum = _bulletNum; x = _x; y = _y; dir = _dir; speed = _speed; pdir = _pdir;
		}
		void update(THUAI4::Character p, double pdir, int ti, bool send)
		{
			update(p.guid, GetJobId(p.jobType), p.hp, p.bulletNum, p.x, p.y, p.facingDirection, p.moveSpeed, pdir, ti);
			if (send)
			{
				stringstream ss;
				ss << "E " << guid << " " << job << " " << hp << " " << bulletNum << " " << x << " " << y << " " << dir << " " << speed << " " << pdir << " " << ti << endl;
				send_message(ss.str());
			}
		}
		bool is_alive() { return hp > 0; }
		int GetTimeSinceRespawn()
		{
			return max(0, GetTime() - last_death_time);
		}
	};
	map<ll, enemy_info > enemy;

	bool NoWall(int x, int y)
	{
		return InMap(x, y) && cell[x][y] != WALL;
	}
	bool CanPaint(int x, int y)
	{
		return InMap(x, y) && can_paint(cell[x][y]);
	}
	void update_cell(int x, int y, int t, int ti = GetTime(), bool send = false)
	{
		if (!CanPaint(x, y)) return;
		Assert(t != UNKNOWN && t >= 0 && t < 8);
		if (ti >= cell_time[x][y])
		{
			cell[x][y] = t;
			cell_time[x][y] = cell_paint_time[x][y] = ti;
			if (send && 1 <= t && t <= 4)
			{
				stringstream ss;
				ss << "C " << x << " " << y << " " << x << " " << y << " " << t << " " << ti;
				send_message(ss.str());
			}
		}
	}

	void update_cells(int xl, int yl, int xr, int yr, int t, int ti = GetTime(), bool send = false)
	{
		for (int x = xl; x <= xr; x++) for (int y = yl; y <= yr; y++) update_cell(x, y, t, ti, send);
	}
	void update_purplefish_cells(GEO::point p, double angle, int ti, bool send = false)
	{
		auto a = PurpleFishCells(p, angle);
		for (auto t : a)
		{
			update_cell(t.first.first, t.first.second, self_color, t.second + ti);
			if (cell[t.first.first][t.first.second] == self_color) cell_paint_time[t.first.first][t.first.second] = ti;
		}
		if (send)
		{
			stringstream ss;
			ss << "CP " << p.x << " " << p.y << " " << angle << " " << ti;
			send_message(ss.str());
		}
	}
	void update_enemy_purplefish_cells(GEO::point p, double angle, int ti)
	{
		auto a = PurpleFishCells(p, angle + pi, 355, 50, false);
		for (auto t : a)
		{
			update_cell(t.first.first, t.first.second, enemy_color, ti);
		}
	}
	void Collect(bool send = true)
	{
		int ti = GetTime();
		self = *G->GetSelfInfo();
		self_p = GEO::point(self.x, self.y);
		self_cx = ToCell(self.x);
		self_cy = ToCell(self.y);
		if (self_color == -1)
		{
			shield_end_time = 5 * 1000;
			self_color = getColorId(G->GetSelfTeamColor());
			enemy_color = self_color ^ 1 ^ 2;
			update_cell(ToCell(self.x), ToCell(self.y), SELFBIRTH);
		}
		score = G->GetTeamScore();
		if (send) send_position();

		auto chars = G->GetCharacters();
		adj_char.clear();
		for (auto p : chars)
		{
			if (p->guid != self.guid)
			{
				adj_char.push_back(*p);
				if (p->teamID != self.teamID)
				{
					if (!p->isDying)
					{
						enemy[p->guid].update(*p, (GEO::point(p->x, p->y) - self_p).rad(), GetTime(), send);
					}
					else
					{
						enemy[p->guid].update_hp(0, GetTime(), send);
					}
				}
			}
		}

		auto walls = G->GetWalls();
		for (auto p : walls)
		{
			int x = ToCell(p->x), y = ToCell(p->y); Assert(InMap(x, y));
			update_cell(x, y, WALL);
		}

		auto births = G->GetBirthPoints();
		for (auto p : births)
		{
			int x = ToCell(p->x), y = ToCell(p->y); Assert(InMap(x, y));
			if (cell[x][y] != SELFBIRTH) update_cell(x, y, BIRTH);
			birth_teamid[x][y] = p->teamID;
		}



		vector<pair<int, int> > adj = AdjCellsToGrid(self.x, self.y, SIGHT_RADIUS * 2);
		for (auto p : adj)
		{
			int i = p.first, j = p.second;
			auto t = G->GetCellColor(i, j);
			if (t == THUAI4::ColorType::Invisible) continue;
			if (GetDist2(self.x, self.y, (p.first + 0.5) * GRID_N, (p.second + 0.5) * GRID_N) < SIGHT_RADIUS * SIGHT_RADIUS / 4) prop_value[i][j] = 0;
			update_cell(i, j, getColorId(t));
		}
		for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
		{
			prop_value[i][j] *= 1 - 1.0 / 1800;
		}

		auto props = G->GetProps();
		adj_prop.clear();
		for (auto p : props)
		{
			if (p->isLaid == true) continue;
			prop_value[ToCell(p->x)][ToCell(p->y)] = 0;
		}
		for (auto p : props)
		{
			adj_prop.push_back(*p);
			//cerr << "Prop at "<<ToCell(p->x)<<","<<ToCell(p->y)<<" with id "<<GetPropId(p->propType) << " isLaid="<<p->isLaid<<endl;
			if (p->isLaid == true) continue;
			prop_value[ToCell(p->x)][ToCell(p->y)] += PropValue[GetPropId(p->propType)];
		}

		auto bullets = G->GetBullets();
		adj_bullet.clear();
		for (auto p : bullets)
		{
			adj_bullet.push_back(*p);
			if (p->bulletType == THUAI4::BulletType::Bullet3 && p->teamID != TEAM_ID)
			{
				update_enemy_purplefish_cells(GEO::point(p->x, p->y), p->facingDirection, ti);
			}
		}


		memset(dism, -1, sizeof(dism));
		dism[self_cx][self_cy] = 0;
		static int q[CELL_N * CELL_N * 4], qn;
		int dx[4] = { 1, -1, 0, 0 };
		int dy[4] = { 0, 0, 1, -1 };
		qn = 0; q[qn++] = self_cx; q[qn++] = self_cy;
		for (int i = 0; i < qn; i += 2)
		{
			int x = q[i], y = q[i + 1];
			for (int d = 0; d < 4; d++)
			{
				int nx = x + dx[d], ny = y + dy[d];
				if (CanMove(nx, ny) && dism[nx][ny] == -1)
				{
					dism[nx][ny] = dism[x][y] + 1;
					q[qn++] = nx; q[qn++] = ny;
				}
			}
		}
		if (send) send_map(ti);
	}

	void DebugMap(int r = 2, bool must = false)
	{
		if (!must && NO_DEBUG_OUT) return;
		cout << "I'm at (" << self.x << "," << self.y << ") - (" << ToCell(self.x) << "," << ToCell(self.y) << ")" << endl;
		cout << "# of Adj char = " << adj_char.size() << endl;
		for (int i = ToCell(self.x) - r; i <= ToCell(self.x) + r; i++)
		{
			if (!InMap(i, 0)) continue;
			for (int j = ToCell(self.y) - r; j <= ToCell(self.y) + r; j++)
			{
				if (InMap(i, j))
				{
					if (cell[i][j] == UNKNOWN) cout << "u";
					else if (cell[i][j] == WALL) cout << "X";
					else if (cell[i][j] == NONE) cout << ".";
					else
					{
						int x = cell[i][j];
						cout << x;
						Assert(x < 8 && x >= 0);
					}
				}
			}
			cout << endl;
		}
	}
	void DebugFullMap(bool must = false) { DebugMap(100, must); }

	//These methods always return true for unknown cells
	int _getd(int l, double x) { return l * GRID_N < x&& x < (l + 1)* GRID_N ? 0 : (x < l* GRID_N ? -1 : 1); }
	bool _intersect(int x, int y, double px, double py, double r)
	{
		int dx = _getd(x, px);
		int dy = _getd(y, py);
		if (dx == 0 && dy == 0) return true;
		if (dy == 0)
		{
			if (_getd(x, px + r) == 0 || _getd(x, px - r) == 0) return true;
		}
		else if (dx == 0)
		{
			if (_getd(y, py + r) == 0 || _getd(y, py - r) == 0) return true;
		}
		else
		{
			if (GetDist2(px, py, (x + (dx + 1) / 2) * GRID_N, (y + (dy + 1) / 2) * GRID_N) < r * r) return true;
		}
		return false;
	}
	bool CanMove(int x, int y)
	{
		if (!InMap(x, y) || !can_move(cell[x][y])) return false;
		for (auto p : adj_char)
		{
			if (_intersect(x, y, p.x, p.y, p.radius)) return false;
		}
		return true;
	}
	bool IsStucked()
	{
		for (int i = -1; i <= 1; i++) for (int j = -1; j <= 1; j++)
		{
			int x = self_cx + i, y = self_cy + j;
			if (!InMap(x, y)) continue;
			if (!CanMove(x, y) && _intersect(x, y, self.x, self.y, self.radius)) return true;
		}
		return false;
	}
	bool CanMoveTo(int x, int y)
	{
		return dism[x][y] != -1;
	}
	bool CanMoveGrid(double x, double y)
	{
		int cx = ToCell(x), cy = ToCell(y);
		return CanMove(ToCell(x), ToCell(y));
	}
	bool CanMoveGrid(GEO::point p) { return CanMoveGrid(p.x, p.y); }
	bool CanMoveCenter(double x, double y)
	{
		int cx = ToCell(x), cy = ToCell(y);
		for (int di = -1; di <= 1; di++) for (int dj = -1; dj <= 1; dj++)
		{
			int i = self_cx + di, j = self_cy + dj;
			if (!InMap(i, j)) continue;
			if (!can_move(cell[i][j]) && _intersect(i, j, x, y, self.radius)) return true;
		}
		for (auto p : adj_char)
		{
			if ((GEO::point(p.x, p.y) - GEO::point(x, y)).len() < self.radius * 2) return false;
		}
		return true;
	}
	bool CanMoveGridSegment(GEO::point p, GEO::point q)
	{
		if (info_debug) cout << "CanMoveGridSegment : " << p.x << "," << p.y << " - " << q.x << "," << q.y << endl;
		if (p == q) return CanMove(ToCell(p.x), ToCell(p.y));
		GEO::point dir = (q - p).unit();
		while (true)
		{
			GEO::line l(p, q);
			GEO::point np;
			int cx = ToCell(p.x), cy = ToCell(p.y);
			bool has = false;
			double d;
			if (abs((q - p).x) > eps)
			{
				for (int i = cx - 1; i <= cx + 1; i++)
				{
					GEO::point ip = p + (q - p) / (q - p).x * (i * GRID_N - p.x);
					if ((ip.x - p.x) * (ip.x - q.x) < eps)
					{
						if (ip == p) continue;
						if (!has || (ip - p).sqrl() < d)
						{
							d = (ip - p).sqrl(), has = true;
						}
					}
				}
			}
			if (abs((q - p).y) > eps)
			{
				for (int j = cy - 1; j <= cy + 1; j++)
				{
					GEO::point ip = p + (q - p) / (q - p).y * (j * GRID_N - p.y);
					if ((ip.y - p.y) * (ip.y - q.y) < eps)
					{
						if (ip == p) continue;
						if (!has || (ip - p).sqrl() < d)
						{
							d = (ip - p).sqrl(), has = true;
						}
					}
				}
			}
			if (!has) break;
			np = p + dir * sqrt(d);
			if (info_debug) cout << ToCell(((p + np) / 2).x) << "," << ToCell(((p + np) / 2).y) << " " << p.x << "," << p.y << " " << np.x << "," << np.y << endl;
			if (!CanMoveGrid((p + np) / 2))
			{
				if (info_debug) cout << "Found false" << endl;
				return false;
			}
			p = np;
		}
		return true;
	}

	bool CanMovePath(GEO::point p, GEO::point q, bool check_points = false)
	{
		//cout << "CanMovePath : " << p.x << "," << p.y << " - " << q.x << "," << q.y << endl;
		if (check_points)
		{
			if (!CanMoveCenter(p.x, p.y)) return false;
			if (!CanMoveCenter(q.x, q.y)) return false;
		}
		if (p == q) return true;
		GEO::point dir = (q - p).unit(), dlt1, dlt2;
		if (USE_CYCLE_SHAPE)
		{
			dlt1 = dir * GEO::point(0, 1) * (self.radius - 0.001);
			dlt2 = dir * GEO::point(0, -1) * (self.radius - 0.001);
			return CanMoveGridSegment(p + dlt1, q + dlt1) && CanMoveGridSegment(p + dlt2, q + dlt2);
		}
		else
		{
			for (int i = -1; i <= 1; i += 2) for (int j = -1; j <= 1; j += 2)
			{
				GEO::point dlt((self.radius - 0.1) * i, (self.radius - 0.1) * j);
				if (!CanMoveGridSegment(p + dlt, q + dlt)) return false;
			}
			return true;
		}
	}

	double PurpleFishSegmentLength(GEO::point p, double angle) //p -> q
	{
		GEO::point dir = GEO::unit_vector(angle), starting_point = p;
		GEO::point q = p + dir * 100 * 1000;
		while (true)
		{
			GEO::line l(p, q);
			GEO::point np;
			int cx = ToCell(p.x), cy = ToCell(p.y);
			bool has = false;
			double d;
			if (abs((q - p).x) > eps)
			{
				for (int i = cx - 1; i <= cx + 1; i++)
				{
					GEO::point ip = p + (q - p) / (q - p).x * (i * GRID_N - p.x);
					if ((ip.x - p.x) * (ip.x - q.x) < eps)
					{
						if (ip == p) continue;
						if (!has || (ip - p).sqrl() < d)
						{
							d = (ip - p).sqrl(), has = true;
						}
					}
				}
			}
			if (abs((q - p).y) > eps)
			{
				for (int j = cy - 1; j <= cy + 1; j++)
				{
					GEO::point ip = p + (q - p) / (q - p).y * (j * GRID_N - p.y);
					if ((ip.y - p.y) * (ip.y - q.y) < eps)
					{
						if (ip == p) continue;
						if (!has || (ip - p).sqrl() < d)
						{
							d = (ip - p).sqrl(), has = true;
						}
					}
				}
			}
			if (!has) break;
			np = p + dir * sqrt(d);
			int ncx = ToCell((p.x + np.x) / 2), ncy = ToCell((p.y + np.y) / 2);
			if (!NoWall(ncx, ncy))
			{
				return (np - starting_point).len();
			}
			p = np;
		}
		Assert(false);
	}
	vector<pair<pair<int, int>, double> > PurpleFishCells(GEO::point p, double angle, int bulletR = 250 * 1.42, int minl = 50, bool blast = true) //p -> q
	{
		vector<pair<pair<int, int>, double> > ret;
		GEO::point dir = GEO::unit_vector(angle), starting_point = p;
		double len = min(PurpleFishSegmentLength(p + dir * GEO::point(0, 1) * bulletR, angle), PurpleFishSegmentLength(p + dir * GEO::point(0, -1) * bulletR, angle));
		GEO::point q = p + dir * len;
		while (true)
		{
			GEO::line l(p, q);
			GEO::point np;
			int cx = ToCell(p.x), cy = ToCell(p.y);
			bool has = false;
			double d;
			if (abs((q - p).x) > eps)
			{
				for (int i = cx - 1; i <= cx + 1; i++)
				{
					GEO::point ip = p + (q - p) / (q - p).x * (i * GRID_N - p.x);
					if ((ip.x - p.x) * (ip.x - q.x) < eps)
					{
						if (ip == p) continue;
						if (!has || (ip - p).sqrl() < d)
						{
							d = (ip - p).sqrl(), has = true;
						}
					}
				}
			}
			if (abs((q - p).y) > eps)
			{
				for (int j = cy - 1; j <= cy + 1; j++)
				{
					GEO::point ip = p + (q - p) / (q - p).y * (j * GRID_N - p.y);
					if ((ip.y - p.y) * (ip.y - q.y) < eps)
					{
						if (ip == p) continue;
						if (!has || (ip - p).sqrl() < d)
						{
							d = (ip - p).sqrl(), has = true;
						}
					}
				}
			}
			if (!has) break;
			np = p + dir * sqrt(d);
			int ncx = ToCell((p.x + np.x) / 2), ncy = ToCell((p.y + np.y) / 2);
			if (!NoWall(ncx, ncy))
			{
				break;
			}
			if ((np - p).len2() > minl * minl)
			{
				ret.push_back(make_pair(make_pair(ncx, ncy), (p - starting_point).len() / 3.0));
			}
			p = np;
		}
		if (ret.size() == 0) return ret;
		static bool hs[CELL_N][CELL_N];
		if (blast)
		{
			for (auto t : ret) hs[t.first.first][t.first.second] = true;
			auto back = ret.back();
			for (int i = -2; i <= 2; i++)
			{
				for (int j = -2; j <= 2; j++)
				{
					int x = back.first.first + i, y = back.first.second + j;
					if (CanPaint(x, y) && !hs[x][y])
					{
						ret.push_back(make_pair(make_pair(x, y), back.second));
					}
				}
			}
			for (auto t : ret) hs[t.first.first][t.first.second] = false;
		}
		return ret;
	}

	int GetCell(int x, int y)
	{
		Assert(InMap(x, y));
		return cell[x][y];
	}
	int GetCurCell()
	{
		return GetCell(self_cx, self_cy);
	}
	bool IsSelfColor(int x, int y) //排除出生点的情况
	{
		return InMap(x, y) && cell[x][y] == self_color;
	}
	bool IsOtherColor(int x, int y) //排除出生点的情况
	{
		return InMap(x, y) && cell[x][y] >= 1 && cell[x][y] <= 4 && cell[x][y] != self_color;
	}
	vector<enemy_info> GetNearbyEnemies(int ti = 55)
	{
		vector<enemy_info> ret;
		for (auto t : enemy)
		{
			auto e = t.second;
			if (e.upd_time + ti > GetTime() && e.is_alive() && (GEO::point(e.x, e.y) - self_p).len() < SIGHT_RADIUS)
			{
				ret.push_back(e);
			}
		}
		return ret;
	}
	bool IsInEmergency()
	{
		return GetNearbyEnemies().size() > 0 && GetTime() > shield_end_time;
	}
	bool IsNearEnemy(int i, int j, int ti, int r, bool include_dead)
	{
		for (auto t : enemy)
		{
			auto e = t.second;
			if (e.upd_time + ti > GetTime() && (include_dead || e.is_alive()) && (GEO::point(e.x, e.y) - CellPos(i, j)).len() < r)
			{
				return true;
			}
		}
		return false;
	}
	bool CanRecover(int i, int j)
	{
		if (IsSelfColor(i, j) && cell_time[i][j] + 100 > GetTime() && GetTime() + 100 > cell_time[i][j])
		{
			if (GetTime() < shield_end_time || !IsNearEnemy(i, j, 5000, SIGHT_RADIUS * 2, true))
			{
				return true;
			}
		}
		return false;
	}
	void GetShield()
	{
		shield_end_time = GetTime() + 30 * 1000;
	}
}Info;


class Executer
{
private:
#define IDLE 0
#define MOVEPLAYER 1
#define SHOOT 2
#define PICK 3
#define USE 4
#define THROW 5
	struct Instruction
	{
		int type;
		vector<double> args;
		int time_duration;
		int state; // Done or not done
		Instruction(int _type, vector<double> _args) {
			type = _type;
			args = _args;
			if (type == IDLE || type == MOVEPLAYER)
			{
				Assert(args.size() > 0);
				time_duration = args[0];
			}
			else time_duration = 0;
		}
		void _Move(int time, double angle)
		{
			G->MovePlayer(time, angle);
		}
		void _Shoot(int time, double angle)
		{
			G->Attack(time, angle);
		}
		void _Pick(int propType)
		{
			G->Pick(PropTypes[propType]);
		}
		void _Use()
		{
			G->Use();
		}
		void _Throw(int time, double angle)
		{
			G->Throw(time, angle);
		}
		void execute()
		{
			if (type == MOVEPLAYER) _Move(args[0], args[1]);
			if (type == SHOOT) _Shoot(args[0], args[1]);
			if (type == PICK) _Pick(args[0]);
			if (type == USE) _Use();
			if (type == THROW) _Throw(args[0], args[1]);
		}
	};
	vector<pair<int, Instruction>> A;
	int pt = 0;												//Current instruction index indicator
	int delay_tot = 0;
	int GetLastStartTime()
	{
		if (A.size() == 0) return GetTime() - 1;
		else return max(A.back().first + delay_tot, GetTime() - 1);
	}
	int GetLastTime()
	{
		if (A.size() == 0) return GetTime() - 1;
		else return max(A.back().first + A.back().second.time_duration + delay_tot, GetTime() - 1);
	}
	void AddEvent(int scheduled_time, Instruction event)
	{
		A.push_back(make_pair(scheduled_time - delay_tot, event));
	}
	void AppendEvent(Instruction event)
	{
		AddEvent(GetLastTime(), event);
	}
public:
	void Execute()
	{
		while (pt < A.size() && A[pt].first + delay_tot <= GetTime())
		{
			if (A[pt].second.time_duration > 0)
			{
				delay_tot += GetTime() - A[pt].first - delay_tot;
			}
			A[pt].second.execute();
			pt++;
		}
	}
	bool IsScheduled()
	{
		Execute();
		return pt < A.size();
	}
	int TimeLeft()    //-1 if no tasks
	{
		return GetLastTime() - GetTime();
	}
	void Idle(int time = DEFAULT_IDLE_TIME)
	{
		while (time > 0)
		{
			int ti = min(time, DEFAULT_IDLE_TIME);
			AppendEvent(Instruction(IDLE, { double(ti) }));
			time -= ti;
		}
	}
	void Move(int time, double angle, bool fangka = true)
	{
		if (!NO_DEBUG_OUT) cout << "Move: time=" << time << " angle=" << angle << " moveSpeed=" << Info.self.moveSpeed << endl;
		if (TIME_TO_CEIL) time = (max(1, time) + 49) / 50 * 50; else time += EXTRA_TIME_FOR_MOVE;
		if (fangka && time * Info.self.moveSpeed / 1000.0 < FANGKA_THRESHOLD)
		{
			time = max(FANGKA_MINIMUM_TIME, time);
			for (int t = -8; t <= 8; t++)
			{
				if (angle > pi / 2 * t + eps && angle < pi / 2 * t + FANGKA_MINIMUM_ANGLE) angle = pi / 2 * t + FANGKA_MINIMUM_ANGLE;
				if (angle < pi / 2 * t - eps && angle > pi / 2 * t - FANGKA_MINIMUM_ANGLE) angle = pi / 2 * t - FANGKA_MINIMUM_ANGLE;
			}
			if (!NO_DEBUG_OUT) cout << "Move after FANGKA: time=" << time << " angle=" << angle << endl;
		}
		while (time > 0)
		{
			int t = min(time, MOVE_DEFAULT_TIME_BLOCK);
			AppendEvent(Instruction(MOVEPLAYER, { double(t), angle }));
			Idle();
			time -= t;
		}
		Execute();
	}
	void Shoot(double angle)
	{
		Assert(playerJob != THUAI4::JobType::Job4 && playerJob != THUAI4::JobType::Job1);
		AppendEvent(Instruction(SHOOT, { 0, angle }));
		Idle(IDLE_TIME_AFTER_ACTION);
		Execute();
	}
	void Shoot(int time, double angle)
	{
		Assert(playerJob == THUAI4::JobType::Job4 || playerJob == THUAI4::JobType::Job1);
		if (TIME_TO_CEIL) time = (time + 49) / 50 * 50;
		AppendEvent(Instruction(SHOOT, { double(time), angle }));
		Idle(IDLE_TIME_AFTER_ACTION);
		Execute();
	}
	void Pick(THUAI4::PropType propType)
	{
		int t = GetPropId(propType);
		AppendEvent(Instruction(PICK, { double(t) }));
		Idle(IDLE_TIME_AFTER_ACTION);
		Execute();
	}
	void Use()
	{
		AppendEvent(Instruction(USE, { }));
		Idle(IDLE_TIME_AFTER_ACTION);
		Execute();
	}
	void Throw(int time, double angle)
	{
		AppendEvent(Instruction(THROW, { double(time), angle }));
		if (TIME_TO_CEIL) time = (time + 49) / 50 * 50;
		Idle(IDLE_TIME_AFTER_ACTION);
		Execute();
	}
	void Clear()
	{
		int t = GetTime();
		while (A.size() > pt && GetLastStartTime() > t) A.pop_back();
	}
}Ex;


class MoveEuclidean
{
private:
	vector < GEO::point > pts;
	vector <double> dis;
	vector <int> pre;
	void add_point(GEO::point p)
	{
		//cout << "addp " << ToCell(p.x) << "," << ToCell(p.y) << endl;
		pts.push_back(p);
	}
public:
	void Prepare(int cx, int cy)
	{
		pts.clear();
		add_point(GEO::point(Info.self.x, Info.self.y));
		int dx[4] = { 1, 1, -1, -1 };
		int dy[4] = { -1, 1, 1, -1 };
		for (int i = 0; i < CELL_N; i++)
		{
			for (int j = 0; j < CELL_N; j++)
			{
				if (!Info.CanMove(i, j)) continue;
				bool flag = false;
				for (int d = 0; d < 4; d++)
				{
					if (!Info.CanMove(i + dx[d], j + dy[d]) && Info.CanMove(i + dx[d], j) && Info.CanMove(i, j + dy[d]))
					{
						flag = true;
						break;
					}
				}
				if (flag)
				{
					add_point(GEO::point(i * GRID_N + GRID_N / 2, j * GRID_N + GRID_N / 2));
				}
			}
		}
	}
	bool Dijkstra(int s, int t)
	{
		dis.clear(); pre.clear();
		int n = pts.size();
		dis.resize(n); pre.resize(n);
		set<pair<double, int> > st;
		vector<int> vis(n, 0);
		for (int i = 0; i < n; i++) dis[i] = i == s ? 0 : 1e100;
		st.insert(make_pair(0, s));
		while (st.size() > 0)
		{
			int x = st.begin()->second;
			st.erase(st.begin());
			if (vis[x]) continue;
			if (x == t) return true;
			vis[x] = true;
			for (int y = 0; y < n; y++)
			{
				if (vis[y]) continue;
				double nd = dis[x] + (pts[y] - pts[x]).len();
				if (!vis[y] && nd < dis[t] && dis[y] > nd && Info.CanMovePath(pts[x], pts[y]))
				{
					dis[y] = nd;
					pre[y] = x;
					st.insert(make_pair(nd, y));
				}
			}
		}
		return false;
	}
	void Escape()
	{
		cout << "Escape at cell (" << Info.self_cx << "," << Info.self_cy << ")" << endl;
		//Info.DebugFullMap(true);
		vector<double> v;
		for (int i = -1; i <= 1; i++) for (int j = -1; j <= 1; j++)
		{
			if (i == 0 && j == 0) continue;
			int x = Info.self_cx + i, y = Info.self_cy + j;
			if (Info.CanMove(x, y)) v.push_back((GEO::point(x, y) - GEO::point(Info.self_cx, Info.self_cy)).rad());
		}
		if (v.size() == 0) v.push_back(1.0 * rand() / RAND_MAX * 2 * pi);
		double d = v[rand() % v.size()];
		Ex.Move(100, d);
	}
	bool MoveToGrid(double x, double y, double time = -1)
	{
		//if (Info.IsStucked()) Escape();
		Prepare(ToCell(x), ToCell(y));
		GEO::point p(x, y);
		if (FANGKA_EXTRA_POINT)
		{
			int cnt = 100;
			while (cnt > 0)
			{
				cnt--;
				GEO::point q = p + GEO::unit_vector(1.0 * rand() / RAND_MAX) * MOVE_RAND_LENGTH;
				if (Info.CanMoveCenter(q.x, q.y))
				{
					add_point(q);
					break;
				}
			}
		}
		add_point(p);
		int s = 0, t = pts.size() - 1;
		if (!Dijkstra(t, s)) return false;
		int cur = s;
		vector<GEO::point> route;
		route.push_back(pts[cur]);
		while (cur != t)
		{
			route.push_back(pts[pre[cur]]);
			cur = pre[cur];
		}
		for (int i = 0; i + 1 < route.size(); i++)
		{
			if (route[i] == route[i + 1]) continue;
			double t = (route[i + 1] - route[i]).len() / Info.self.moveSpeed * 1000;
			if (time < eps) break;
			Ex.Move(min(t, time), (route[i + 1] - route[i]).rad());
			time -= min(t, time);
		}
		if (!NO_DEBUG_OUT)
		{
			for (int i = 0; i < route.size(); i++)
			{
				cout << route[i].x << "," << route[i].y;
				if (i + 1 < route.size()) cout << " -> ";
			}cout << endl;
		}
		return true;
	}
	bool MoveToCell(int x, int y, double time = -1)
	{
		return MoveToGrid((x + 0.5) * GRID_N, (y + 0.5) * GRID_N, time);
	}

}MoveE;
class MoveManhattan
{
private:
	int dx[4] = { 1, -1, 0, 0 };
	int dy[4] = { 0, 0, 1, -1 };
	int dis[CELL_N][CELL_N];
	int mnl, tx, ty;
	bool vis[CELL_N][CELL_N];
	vector<pair<int, int> > path;
public:
	int cur_ti;
	bool dfs(int x, int y, bool out)
	{
		if (!(Info.CanRecover(x, y))) return false;
		vis[x][y] = true;
		if (dis[x][y] < mnl)
		{
			mnl = dis[x][y];
			tx = x; ty = y;
		}
		if (out && dis[x][y] == mnl)
		{
			path.push_back(make_pair(x, y));
			return true;
		}
		for (int d = 0; d < 4; d++)
		{
			int nx = x + dx[d], ny = y + dy[d];
			if (InMap(nx, ny) && dis[nx][ny] == dis[x][y] - 1 && dis[nx][ny] != -1)
			{
				if (!vis[nx][ny] && dfs(nx, ny, out))
				{
					path.push_back(make_pair(x, y));
					return true;
				}
			}
		}
		return false;
	}
	void bfs(int sx, int sy)
	{
		memset(dis, -1, sizeof(dis));
		dis[sx][sy] = 0;
		static int q[CELL_N * CELL_N * 2], qn;
		qn = 0;
		q[qn++] = sx; q[qn++] = sy;
		for (int i = 0; i < qn; i += 2)
		{
			int x = q[i], y = q[i + 1];
			for (int d = 0; d < 4; d++)
			{
				int nx = x + dx[d], ny = y + dy[d];
				if (Info.CanMoveTo(nx, ny) && dis[nx][ny] == -1)
				{
					dis[nx][ny] = dis[x][y] + 1;
					q[qn++] = nx; q[qn++] = ny;
				}
			}
		}
	}
	bool MoveToCell(int x, int y, double time = MOVE_DEFAULT_TIME_BLOCK)
	{
		if (!Info.CanMoveTo(x, y)) return false;
		bfs(x, y);
		int cx = Info.self_cx, cy = Info.self_cy;
		mnl = dis[cx][cy]; tx = -1;
		path.clear();
		if (mnl == -1) return false;

		cur_ti = GetTime();
		memset(vis, 0, sizeof(vis));
		dfs(cx, cy, false);
		if (tx == -1) return false;
		memset(vis, 0, sizeof(vis));
		dfs(cx, cy, true);

		//cerr << "Manhattan MoveToCell:" << x << "," << y << " " << time << endl;
		vector<GEO::point> route;
		route.push_back(Info.self_p);
		path.pop_back();
		reverse(path.begin(), path.end());
		for (auto t : path) route.push_back(CellPos(t.first, t.second));
		for (int i = 0; i + 1 < route.size(); i++)
		{
			if (route[i] == route[i + 1]) continue;
			double t = (route[i + 1] - route[i]).len() / Info.self.moveSpeed * 1000;
			if (time < eps) break;
			t = t + 2;
			Ex.Move(min(t, time), (route[i + 1] - route[i]).rad(), false);
			time -= min(t, time);
		}
		return true;
	}
}MoveM;





































//Player Logic Begins
string state;
int play_count = 0;

double make01(double x) { return min(1.0, max(0.0, x)); }
double makelr(double l, double r, double x) { return abs(r - l) < eps ? (x < l ? 0 : 1) : make01((x - l) / (r - l)); }
double _sum[CELL_N][CELL_N];

double get_sum(int x, int y) { return x < 0 || y < 0 ? 0 : _sum[min(x, CELL_N - 1)][min(y, CELL_N - 1)]; }
double get_sum(int xl, int yl, int xr, int yr) { return get_sum(xr, yr) - get_sum(xl - 1, yr) - get_sum(xr, yl - 1) + get_sum(xl - 1, yl - 1); }
void make2Dsum(double w[CELL_N][CELL_N], int r)
{
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) _sum[i][j] = w[i][j];
	for (int i = 1; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) _sum[i][j] += _sum[i - 1][j];
	for (int i = 0; i < CELL_N; i++) for (int j = 1; j < CELL_N; j++) _sum[i][j] += _sum[i][j - 1];
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) w[i][j] = get_sum(i - r, j - r, i + r, j + r);
}

void add_persistent_value(double w[CELL_N][CELL_N], double value[CELL_N][CELL_N])
{
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) w[i][j] += value[i][j];
}
void add_seen_value(double w[CELL_N][CELL_N], double unseen_coef, double enemy_coef, double ally_coef)
{
	int cur_time = GetTime();
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
	{
		w[i][j] += ((Info.cell[i][j] == UNKNOWN) ? 4 * makelr(0, 300 * 1000, cur_time) : makelr(0, 60 * 1000, cur_time - Info.cell_paint_time[i][j])) * unseen_coef;

		double enemy_val = 0;
		for (auto t : Info.enemy)
		{
			auto& e = t.second;
			if (e.is_alive())
			{
				double R = max(1.0, max(0, cur_time - e.upd_time) / 1000.0 * e.speed);
				if (R > 2) continue;
				enemy_val += (1 - makelr(1, R, (GEO::point(e.x, e.y) - GEO::point((i + 0.5) * 1000, (j + 0.5) * 1000)).len())) / (4 * R * R);
			}
		}
		w[i][j] += enemy_val * enemy_coef;
	}
	for (int i = 0; i < PLAYER_NUM; i++)
	{
		if (i == PLAYER_ID) continue;
		Assert(InMap(Info.team_pos[i].first, Info.team_pos[i].second));
		w[Info.team_pos[i].first][Info.team_pos[i].second] -= ally_coef;
	}
}
void add_prop_value(double w[CELL_N][CELL_N])
{
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
	{
		if (Info.CanMoveTo(i, j))
		{
			w[i][j] += Info.prop_value[i][j] * (1 - makelr(0, 10, Info.dism[i][j]) * 0.5);
		}
	}
}
void adjust_move_value(double w[CELL_N][CELL_N], pair<int, int> last_targ)
{
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
	{
		double coef = 2 - makelr(5, 15, Info.dism[i][j]) + (make_pair(i, j) == last_targ);
		coef *= Info.CanMoveTo(i, j);
		w[i][j] *= coef;
	}
}



void add_cell_shoot_value(double w[CELL_N][CELL_N], double blank_value, double dis_coef)
{
	int cur_time = GetTime();
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
	{
		double decay = 1 - makelr(0, 120 * 1000, cur_time - Info.cell_paint_time[i][j]);
		double v;
		if (!Info.CanMoveTo(i, j)) v = 0;
		else
		{
			if (Info.IsSelfColor(i, j)) v = 1 - decay;
			else if (Info.IsOtherColor(i, j)) v = 2;
			else v = blank_value - makelr(0, 300 * 1000, cur_time);
			if (!Info.IsSelfColor(i, j)) v += dis_coef * (1 - makelr(3, 6, Info.dism[i][j])) * (1 - 0.5 * makelr(540 * 1000, 600 * 1000, cur_time));
		}
		w[i][j] += v;
	}
}
/*
void add_enemy_color_value(double w[CELL_N][CELL_N], int r, double predict_coef, double aid_coef, double enemy_val)
{
	int cur_time = GetTime();
	for (auto t : Info.enemy)
	{
		auto& e = t.second;
		if (!e.is_alive() || GetTime() < 5 * 1000) continue;
		for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
		{
			double v = 0;
			GEO::point p((i + 0.5) * GRID_N, (j + 0.5) * GRID_N);
			double delay = (p - Info.self_p).len() / BULLET_VELOCITY * 1000; //in miliseconds
			GEO::point ep(e.x, e.y);
			ep = ep + GEO::unit_vector(e.dir) * delay * predict_coef;

			double R = max(1.0, max(0.0, cur_time * aid_coef + delay - e.upd_time) / 1000.0 * e.speed / GRID_N);
			if (R > 10) continue;
			for (int dx = -r; dx <= r; dx++) for (int dy = -r; dy <= r; dy++)
			{
				int x = i + dx, y = j + dy;
				v += Info.IsOtherColor(x, y) * (1 - makelr(1, R, (ep - CellPos(x, y)).len() / GRID_N));
			}
			v /= pi / 3 * R * R;
			w[i][j] += v * enemy_val;
		}
	}
}
*/
void add_enemy_color_value(double w[CELL_N][CELL_N], int r, double enemy_val)
{
	int cur_time = GetTime();
	for (auto t : Info.enemy)
	{
		auto& e = t.second;
		if (!e.is_alive()) continue;
		int ex = ToCell(e.x), ey = ToCell(e.y);
		for (int x = ex - r; x <= ex + r; x++) for (int y = ey - r; y <= ey + r; y++)
		{
			if (InMap(x, y)) w[x][y] += enemy_val;
		}
	}
}
void add_enemy_spawn_value(double w[CELL_N][CELL_N], double val, bool all_time)
{
	int cur_time = GetTime();
	for (auto t : Info.enemy)
	{
		auto& e = t.second;
		int ti = e.GetTimeSinceRespawn();
		if (ti > 28 * 1000 && ti < 35 * 1000)
		{
			auto pos = guid_to_spawn[e.guid];
			auto cells = AdjCellsToGrid((pos.first + 0.5) * GRID_N, (pos.second + 0.5) * GRID_N, 20 * 1000);
			for (auto p : cells)
			{
				if (InMap(p.first, p.second)) w[p.first][p.second] += val;
			}
		}
	}
}
pair<int, int> find_max(double w[CELL_N][CELL_N], double base, bool move_to)
{
	int mxi = -1, mxj = -1; double mx = base;
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
	{
		if ((!move_to || Info.CanMoveTo(i, j)) && w[i][j] > mx)
		{
			mxi = i; mxj = j; mx = w[i][j];
		}
	}
	return make_pair(mxi, mxj);
}


int cell_enemy_hp[CELL_N][CELL_N]; vector<ll> cell_enemies[CELL_N][CELL_N];
void add_kill_enemy_value(double w[CELL_N][CELL_N], int r, double predict_coef, double aid_coef, double shoot_threshold, double val, int shoot_type = rand() % 2)
{
	memset(cell_enemy_hp, 0, sizeof(cell_enemy_hp));
	for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) cell_enemies[i][j].clear();

	int cur_time = GetTime();
	for (auto t : Info.enemy)
	{
		auto& e = t.second;
		if (!e.is_alive() || GetTime() < 5 * 1000) continue;
		double delay = (GEO::point(e.x, e.y) - Info.self_p).len() / BULLET_VELOCITY * 1000; //in miliseconds
		double R = max(1.0, max(0.0, delay * aid_coef + cur_time - e.upd_time) / 1000.0 * e.speed / GRID_N);
		GEO::point p(e.x, e.y);
		bool near = (GEO::point(e.x, e.y) - Info.self_p).len() < SIGHT_RADIUS * 1.2;
		int type = shoot_type;
		if (near && rand() % 2) type = 1;
		if (type == 0)
		{
			p = p + GEO::unit_vector(e.dir) * e.speed * delay / 1000 * predict_coef * (0.75 + 0.5 * rand() / RAND_MAX);
		}
		else if (type == 1)
		{
			p = p + GEO::unit_vector(e.pdir) * e.speed * delay / 1000 * predict_coef * (1.5 + rand() % 2);
		}
		int cx = ToCell(p.x), cy = ToCell(p.y);
		if (R <= shoot_threshold)
		{
			w[cx][cy] += val;
			cell_enemy_hp[cx][cy] = max(cell_enemy_hp[cx][cy], e.hp);
			cell_enemies[cx][cy].push_back(e.guid);
		}
	}
}

bool CAN_SHOOT_TO_RECOVER;

class RecoverScheme
{
public:

	bool moving_to_recover = false;
	bool starting_recover = false;

	void go(pair<int, int> targ, bool move)
	{
		bool can_recover = Info.CanRecover(Info.self_cx, Info.self_cy);
		if (Ex.TimeLeft() > 50)
		{
			if (!starting_recover && !moving_to_recover) Ex.Clear(); //刚开始想要回子弹的时候清空所有动作
			if (starting_recover && !moving_to_recover && !can_recover) //自己脚下颜色改变的时候中止回子弹
			{
				Ex.Clear();
				starting_recover = false;
			}
			return;
		}

		if (can_recover)
		{
			if (!starting_recover)
			{
				starting_recover = true;
				moving_to_recover = false;
			}
			else if (move)
			{
				MoveM.MoveToCell(targ.first, targ.second);
			}
		}
		else
		{
			double mndis = 1e9;
			int mni = Info.self_birth_cx, mnj = Info.self_birth_cy, ti = GetTime();
			for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++)
			{
				if (Info.CanMoveTo(i, j) && Info.CanRecover(i, j))
				{
					double d = Info.dism[i][j] * (1 + makelr(0, 60 * 1000, ti - Info.cell_paint_time[i][j]) * 5);
					if (d < mndis)
					{
						mndis = d;
						mni = i; mnj = j;
					}
				}
			}
			if (CAN_SHOOT_TO_RECOVER && Info.CanRecover(Info.self_cx, Info.self_cy) && Info.self.bulletNum >= 1 && mndis > 3)
			{
				if (playerJob == THUAI4::JobType::Job3) Ex.Shoot(pi / 2);
				else Ex.Shoot(0, 0);
			}
			else
			{
				MoveE.MoveToCell(mni, mnj, MOVE_DEFAULT_TIME_BLOCK * 3);
				moving_to_recover = true;
			}
		}
	}
	void start()
	{
		starting_recover = false;
		moving_to_recover = false;
	}
}RecoverBullet;

void CheckProp(pair<int, int> last_targ)
{
	if (make_pair(Info.self_cx, Info.self_cy) == last_targ)
	{
		for (auto& p : Info.adj_prop)
		{
			if (!p.isLaid && Info.self_cx == ToCell(p.x) && Info.self_cy == ToCell(p.y)) //能被捡起
			{
				if (Ex.TimeLeft() >= 100) return;
				Ex.Pick(p.propType);
				Ex.Use();
				if (p.propType == THUAI4::PropType::Shield) Info.GetShield();
			}
		}
	}
}


namespace JOB1
{
	const int MOVE_ACTION_TIME = MOVE_DEFAULT_TIME_BLOCK * 3;
	const int MAX_BULLET_NUM_IN_ONE_SHOOT = 4;
	const int THRESHOLD_FOR_MAX_BULLET = 9;
	const int THRESHOLD_FOR_MIN_BULLET = 1;
	const int THRESHOLD_TO_START_RECOVERING = 4;
	const int THRESHOLD_TO_END_RECOVERING = 8;
	const int THRESHOLD_TO_MOVE_IN_RECOVERING = 4;
	const int AID_COEFFICIENT = 0.66;
	const int SHOOT_THRESHOLD = 4;
	const int PREDICT_COEFFICIENT = 0.15;
	class MoveScheme
	{
	public:
		struct MoveMatrix
		{
			double value[CELL_N][CELL_N]; //persistent value
			vector<pair<int, int> > failed_cell;

			void init()
			{
				last_targ = make_pair(-1, -1);
				for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) value[i][j] = 0;
			}

			pair<int, int> last_targ;
			pair<int, int> GetTarget(int r = 4)
			{
				static double w[CELL_N][CELL_N];
				memset(w, 0, sizeof(w));
				//seen value
				add_seen_value(w, 1, 0, 10);
				make2Dsum(w, r);

				add_persistent_value(w, value);
				add_prop_value(w);
				for (auto p : failed_cell) w[p.first][p.second] += -1e9;
				add_enemy_spawn_value(w, -200, false);
				adjust_move_value(w, last_targ);
				return last_targ = find_max(w, -1e100, true);
			}
		}M;
		void init()
		{
			M.init();
			GEO::point dir;
			if (PLAYER_ID == 0) dir = GEO::point(-1, -1);
			if (PLAYER_ID == 1) dir = GEO::point(1, -1);
			if (PLAYER_ID == 2) dir = GEO::point(-1, 1);
			if (PLAYER_ID == 3) dir = GEO::point(1, 1);
			GEO::point O(CELL_N / 2, CELL_N / 2);
			for (int i = 0; i < CELL_N; i++)
			{
				for (int j = 0; j < CELL_N; j++)
				{
					double t = (GEO::point(i, j) - O).dot(dir) / CELL_N;
					M.value[i][j] += (t + 1) / 2 * 5;
				}
			}
		}
		void go()
		{
			int retry_times = 5;
			while (true)
			{
				auto targ = M.GetTarget();
				if (retry_times > 0 && !MoveE.MoveToCell(targ.first, targ.second, MOVE_ACTION_TIME))
				{
					retry_times--;
					cout << "Move to (" << targ.first << "," << targ.second << ") failed" << endl;
					M.failed_cell.push_back(targ);
				}
				else
				{
					if (!NO_DEBUG_OUT) cout << "Move to " << targ.first << "," << targ.second << endl;
					break;
				}
			}
			//	if (retry_times < 5) Info.DebugFullMap();
			M.failed_cell.clear();
		}
	}Move;
	class ShootScheme
	{
	public:
		struct ShootMatrix
		{
			double value[CELL_N][CELL_N]; //persistent
			void init()
			{
				for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) value[i][j] = 0;
			}
			pair<int, int> GetTarget(bool must, int r = BULLET_PAINT_R)
			{
				static double w[CELL_N][CELL_N];
				memset(w, 0, sizeof(w));
				add_persistent_value(w, value);
				add_cell_shoot_value(w, 2, 0.5);
				make2Dsum(w, r);

				add_enemy_color_value(w, 5, 3);
				add_kill_enemy_value(w, r, PREDICT_COEFFICIENT, AID_COEFFICIENT, SHOOT_THRESHOLD, 500);
				if (must) return find_max(w, -1e9, false);
				else return find_max(w, eps, false);
			}
		}M;
		void init()
		{
			M.init();
		}

		void go(bool must = false)
		{
			if (Info.self.bulletNum == 0) return;
			auto targ = M.GetTarget(must); int tx = targ.first, ty = targ.second;
			if (tx == -1) return;
			GEO::point p = CellPos(tx, ty) - Info.self_p;
			double dlt_time = p.len() / BULLET_VELOCITY * 1000;
			Info.update_cells(targ.first - BULLET_PAINT_R, targ.second - BULLET_PAINT_R,
				targ.first + BULLET_PAINT_R, targ.second + BULLET_PAINT_R, Info.self_color, GetTime() + dlt_time, true);
			if (!NO_DEBUG_OUT) cout << "Shoot:" << targ.first << "," << targ.second << endl;
			if (cell_enemies[tx][ty].size() == 0) Ex.Shoot(dlt_time, p.rad());
			else
			{
				int cnt = min(MAX_BULLET_NUM_IN_ONE_SHOOT, int((cell_enemy_hp[tx][ty] + Info.self.ap - 1) / Info.self.ap));
				set<ll> guids;
				for (auto id : cell_enemies[tx][ty]) guids.insert(id);
				for (int i = 0; i < cnt; i++)
				{
					Ex.Shoot(dlt_time, p.rad());
				}
				for (auto id : guids) Info.enemy[id].update_hp(Info.enemy[id].hp - cnt * Info.self.ap, GetTime() + dlt_time, true);
			}
		}

	}Shoot;



	void Initialize()
	{
		Move.init();
		Shoot.init();
	}
	void Commander()
	{
		if (play_count == 0) Initialize();
		if (Info.self.isDying)
		{
			if (state != "DYING")
			{
				cout << "##### I'm dying! #####" << endl;
				Ex.Clear();
				state = "DYING";
			}
			return;
		}
		if (state == "DYING")
		{
			state = "";
			cout << "##### I'm back! #####" << endl;
			return;
		}
		if (Info.IsInEmergency())
		{
			if (Info.self.bulletNum > THRESHOLD_FOR_MIN_BULLET)
			{
				if (state != "EMERGENCY")
				{
					state = "EMERGENCY";
					Ex.Clear();
					cerr << "IN EMERGENCY" << endl;
				}
				Shoot.go(true);
			}
			else
			{
				state = "RECOVERING";
				RecoverBullet.start();
			}
		}
		else
		{
			if (state == "EMERGENCY") state = "";
		}
		bool can_move_in_recovering = Info.self.bulletNum >= THRESHOLD_TO_MOVE_IN_RECOVERING;
		if (state == "RECOVERING")
		{
			if (Info.self.bulletNum >= THRESHOLD_TO_END_RECOVERING)
			{
				state = "";
				Ex.Clear();
			}
			else
			{
				RecoverBullet.go(Move.M.GetTarget(), can_move_in_recovering);
				return;
			}
		}
		if (state == "")
		{
			if (Info.self.bulletNum <= THRESHOLD_TO_START_RECOVERING)
			{
				state = "RECOVERING";
				RecoverBullet.start();
				RecoverBullet.go(Move.M.GetTarget(), can_move_in_recovering);
				return;
			}
			CheckProp(Move.M.last_targ);
			if (Ex.TimeLeft() < 10)
			{
				if (Info.IsStucked())
				{
					MoveE.Escape();
				}
				else
				{
					int cnt = max(1, Info.self.bulletNum - THRESHOLD_FOR_MAX_BULLET);
					while (cnt--) Shoot.go();
					Move.go();
				}
			}
		}

		if (!NO_DEBUG_OUT && play_count % 5 == 0) cout << "I'm at (" << Info.self.x << "," << Info.self.y << ") bulletNum = " << Info.self.bulletNum << endl;
	}
};


namespace JOB4
{
	const int MOVE_ACTION_TIME = MOVE_DEFAULT_TIME_BLOCK * 2;
	const bool REMOVE_SHOOT_SCHEME = false;
	const double AID_COEFFICIENT = 0.5; //愿意帮助队友的程度 越小越愿意
	const double SHOOT_THRESHOLD = 5.5; //在范围内则发射子弹
	const double PREDICT_COEFFICIENT = 0.2; //预判量
	const int MAX_BULLET_NUM_IN_ONE_SHOOT = 3;

	const int THRESHOLD_FOR_MAX_BULLET = 3;
	const int THRESHOLD_FOR_MIN_BULLET = 0;
	const int THRESHOLD_TO_START_RECOVERING = 1;
	const int THRESHOLD_TO_END_RECOVERING = 3;
	const int THRESHOLD_TO_MOVE_IN_RECOVERING = 2;
	class MoveScheme
	{
	public:
		struct MoveMatrix
		{
			double value[CELL_N][CELL_N]; //persistent value
			vector<pair<int, int> > failed_cell;

			void init()
			{
				last_targ = make_pair(-1, -1);
				for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) value[i][j] = 0;
			}

			pair<int, int> last_targ;
			pair<int, int> GetTarget(int r = 4)
			{
				static double w[CELL_N][CELL_N];
				memset(w, 0, sizeof(w));
				//seen value
				add_seen_value(w, 1, 100, 10);
				make2Dsum(w, r);

				add_persistent_value(w, value);
				add_prop_value(w);
				for (auto p : failed_cell) w[p.first][p.second] += -1e9;
				add_enemy_spawn_value(w, -200, false);
				adjust_move_value(w, last_targ);

				return last_targ = find_max(w, -1e100, true);
			}
		}M;

		void init()
		{
			M.init();
			GEO::point dir;
			if (PLAYER_ID == 0) dir = GEO::point(-1, -1);
			if (PLAYER_ID == 1) dir = GEO::point(1, -1);
			if (PLAYER_ID == 2) dir = GEO::point(-1, 1);
			if (PLAYER_ID == 3) dir = GEO::point(1, 1);
			/*
			GEO::point O(CELL_N / 2, CELL_N / 2);
			for (int i = 0; i < CELL_N; i++)
			{
				for (int j = 0; j < CELL_N; j++)
				{
					double t = (GEO::point(i, j) - O).dot(dir) / CELL_N;
					M.value[i][j] += (t + 1) / 2 * 5;
				}
			}
			*/
			for (int i = 0; i < CELL_N; i++)
			{
				for (int j = 0; j < CELL_N; j++)
				{
					M.value[i][j] += (1 - makelr(0, 25, min(min(j, CELL_N - j), min(i, CELL_N - i)))) * 5;
				}
			}

		}
		void go()
		{
			int retry_times = 5;
			while (true)
			{
				auto targ = M.GetTarget();
				if (retry_times > 0 && !MoveE.MoveToCell(targ.first, targ.second, MOVE_ACTION_TIME))
				{
					retry_times--;
					cout << "Move to (" << targ.first << "," << targ.second << ") failed" << endl;
					M.failed_cell.push_back(targ);
				}
				else
				{
					if (!NO_DEBUG_OUT) cout << "Move to (" << targ.first << "," << targ.second << ")" << endl;
					break;
				}
			}
			//	if (retry_times < 5) Info.DebugFullMap();
			M.failed_cell.clear();
		}
	}Move;
	class ShootScheme
	{
	public:
		struct ShootMatrix
		{
			double value[CELL_N][CELL_N];
			ShootMatrix()
			{
				for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) value[i][j] = 0;
			}

			pair<int, int> GetTarget(bool must, int r = 3)
			{
				static double w[CELL_N][CELL_N];
				int cur_time = GetTime();

				memset(w, 0, sizeof(w));
				add_persistent_value(w, value);
				add_kill_enemy_value(w, r, PREDICT_COEFFICIENT, AID_COEFFICIENT, SHOOT_THRESHOLD, 1);
				if (must) return find_max(w, -1e9, false);
				else return find_max(w, eps, false);
			}
		}M;
		void init()
		{

		}
		bool can_shoot()
		{
			return Info.self.bulletNum > 0 && M.GetTarget(false).first != -1;
		}
		void go(bool must = false)
		{
			if (REMOVE_SHOOT_SCHEME) return;
			if (Info.self.bulletNum == 0) return;
			auto targ = M.GetTarget(must);
			int tx = targ.first, ty = targ.second;
			if (tx == -1) return;
			GEO::point p = CellPos(tx, ty) - Info.self_p;
			double dlt_time = p.len() / BULLET_VELOCITY * 1000;
			Info.update_cells(targ.first - BULLET_PAINT_R, targ.second - BULLET_PAINT_R,
				targ.first + BULLET_PAINT_R, targ.second + BULLET_PAINT_R, Info.self_color, GetTime() + dlt_time, true);

			if (!NO_DEBUG_OUT) cout << "Shoot:" << tx << "," << ty << endl;

			int cnt = min(MAX_BULLET_NUM_IN_ONE_SHOOT, int((cell_enemy_hp[tx][ty] + Info.self.ap - 1) / Info.self.ap));
			set<ll> guids;
			for (auto id : cell_enemies[tx][ty]) guids.insert(id);
			for (int i = 0; i < cnt; i++)
			{
				Ex.Shoot(dlt_time, p.rad());
			}
			for (auto id : guids) Info.enemy[id].update_hp(Info.enemy[id].hp - cnt * Info.self.ap, GetTime() + dlt_time, true);
		}
	}Shoot;



	void Initialize()
	{
		Move.init();
		Shoot.init();
	}

	void Commander()
	{
		if (play_count == 0) Initialize();
		if (Info.self.isDying)
		{
			if (state != "DYING")
			{
				cout << "##### I'm dying! #####" << endl;
				Ex.Clear();
				state = "DYING";
			}
			return;
		}
		if (state == "DYING")
		{
			state = "";
			cout << "##### I'm back! #####" << endl;
			return;
		}
		if (Info.IsInEmergency())
		{
			if (Info.self.bulletNum > THRESHOLD_FOR_MIN_BULLET)
			{
				if (state != "EMERGENCY")
				{
					state = "EMERGENCY";
					Ex.Clear();
				}
				Shoot.go(true);
			}
			else
			{
				state = "RECOVERING";
				RecoverBullet.start();
			}
		}
		else
		{
			if (state == "EMERGENCY") state = "";
		}

		bool can_move_in_recovering = Info.self.bulletNum >= THRESHOLD_TO_MOVE_IN_RECOVERING;
		bool can_shoot = Shoot.can_shoot();
		if (state == "RECOVERING")
		{
			if (can_shoot)
			{
				state = "";
				Ex.Clear();
			}
			else
			{
				if (Info.self.bulletNum >= THRESHOLD_TO_END_RECOVERING)
				{
					state = "";
					Ex.Clear();
				}
				else
				{
					RecoverBullet.go(Move.M.GetTarget(), can_move_in_recovering);
					return;
				}
			}
		}
		if (state == "")
		{
			if (Info.self.bulletNum <= 0 || !can_shoot && Info.self.bulletNum <= THRESHOLD_TO_START_RECOVERING)
			{
				state = "RECOVERING";
				RecoverBullet.start();
				RecoverBullet.go(Move.M.GetTarget(), can_move_in_recovering);
				return;
			}
			CheckProp(Move.M.last_targ);
			if (Ex.TimeLeft() < 10)
			{
				if (Info.IsStucked())
				{
					MoveE.Escape();
				}
				else
				{
					int cnt = max(1, Info.self.bulletNum - THRESHOLD_FOR_MAX_BULLET);
					while (cnt--) Shoot.go();
					Move.go();
				}
			}
		}

		if (!NO_DEBUG_OUT && play_count % 5 == 0) cout << "I'm at (" << Info.self.x << "," << Info.self.y << ") bulletNum = " << Info.self.bulletNum << endl;
	}
};


namespace JOB3
{
	const int MOVE_ACTION_TIME = MOVE_DEFAULT_TIME_BLOCK * 6;
	const int TRY_LINES = 200;

	const int THRESHOLD_FOR_MAX_BULLET = 4;
	const int THRESHOLD_FOR_MIN_BULLET = 1;
	const int THRESHOLD_TO_START_RECOVERING = 1;
	const int THRESHOLD_TO_END_RECOVERING = 4;
	const int THRESHOLD_TO_MOVE_IN_RECOVERING = 1;
	const bool MOVE_WHEN_RECOVERING = false;
	class MoveScheme
	{
	public:
		struct MoveMatrix
		{
			double value[CELL_N][CELL_N]; //persistent value
			vector<pair<int, int> > failed_cell;

			void init()
			{
				last_targ = make_pair(-1, -1);
				for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) value[i][j] = 0;
			}

			pair<int, int> last_targ;
			pair<int, int> GetTarget(int r = 4)
			{
				static double w[CELL_N][CELL_N];
				memset(w, 0, sizeof(w));
				//seen value
				add_seen_value(w, 1, 0, 10);
				make2Dsum(w, r);

				add_persistent_value(w, value);
				add_prop_value(w);
				for (auto p : failed_cell) w[p.first][p.second] += -1e9;
				add_enemy_spawn_value(w, -300, true);
				adjust_move_value(w, last_targ);

				return last_targ = find_max(w, -1e100, true);
			}
		}M;
		void init()
		{
			M.init();
			GEO::point dir;
			if (PLAYER_ID == 0) dir = GEO::point(-1, -1);
			if (PLAYER_ID == 1) dir = GEO::point(1, -1);
			if (PLAYER_ID == 2) dir = GEO::point(-1, 1);
			if (PLAYER_ID == 3) dir = GEO::point(1, 1);
			GEO::point O(CELL_N / 2, CELL_N / 2);
			for (int i = 0; i < CELL_N; i++)
			{
				for (int j = 0; j < CELL_N; j++)
				{
					double t = (GEO::point(i, j) - O).dot(dir) / CELL_N;
					M.value[i][j] += (t + 1) / 2 * 5;
				}
			}
		}
		void go()
		{
			int retry_times = 5;
			while (true)
			{
				auto targ = M.GetTarget();
				if (retry_times > 0 && !MoveE.MoveToCell(targ.first, targ.second, MOVE_ACTION_TIME))
				{
					retry_times--;
					cout << "Move to (" << targ.first << "," << targ.second << ") failed" << endl;
					M.failed_cell.push_back(targ);
				}
				else
				{
					if (!NO_DEBUG_OUT) cout << "Move to " << targ.first << "," << targ.second << endl;
					break;
				}
			}
			//	if (retry_times < 5) Info.DebugFullMap();
			M.failed_cell.clear();
		}
	}Move;
	class ShootScheme
	{
	public:
		struct ShootMatrix
		{
			double value[CELL_N][CELL_N]; //persistent
			void init()
			{
				for (int i = 0; i < CELL_N; i++) for (int j = 0; j < CELL_N; j++) value[i][j] = 0;
			}

			double GetTarget(bool must)
			{
				double threshold = 100 * (1 - makelr(2, 6, Info.self.bulletNum)) * (1 - makelr(600, 2000, Info.score));
				static double w[CELL_N][CELL_N];
				memset(w, 0, sizeof(w));
				add_persistent_value(w, value);
				add_cell_shoot_value(w, 3, 0.5);
				double mx = 0, mxd = 1e9;
				if (must) mx = -1e9, mxd = 0;
				for (int i = 0; i < TRY_LINES; i++)
				{
					double d = 2 * pi / TRY_LINES * i;
					auto t = Info.PurpleFishCells(Info.self_p, d);
					double val = 0;
					for (auto c : t)
					{
						val += w[c.first.first][c.first.second];
					}
					val -= threshold;
					if (val > mx)
					{
						mx = val;
						mxd = d;
					}
				}
				return mxd;
			}
		}M;
		void init()
		{
			M.init();
		}

		void go(bool must = false)
		{
			if (Info.self.bulletNum == 0) return;
			double targ = M.GetTarget(must);
			if (targ > 1e8) return;
			Info.update_purplefish_cells(Info.self_p, targ, GetTime(), true);
			if (!NO_DEBUG_OUT) cout << "Shoot:" << targ << endl;
			Ex.Shoot(targ);
		}

	}Shoot;



	void Initialize()
	{
		Move.init();
		Shoot.init();
	}

	void Commander()
	{
		if (play_count == 0) Initialize();

		if (Info.self.isDying)
		{
			if (state != "DYING")
			{
				cout << "##### I'm dying! #####" << endl;
				Ex.Clear();
				state = "DYING";
			}
			return;
		}
		if (state == "DYING")
		{
			state = "";
			cout << "##### I'm back! #####" << endl;
			return;
		}
		if (Info.IsInEmergency())
		{
			if (Info.self.bulletNum > THRESHOLD_FOR_MIN_BULLET)
			{
				if (state != "EMERGENCY")
				{
					state = "EMERGENCY";
					Ex.Clear();
				}
				Shoot.go(true);
			}
			else
			{
				state = "RECOVERING";
				RecoverBullet.start();
			}
		}
		else
		{
			if (state == "EMERGENCY") state = "";
		}
		bool can_move_in_recovering = Info.self.bulletNum >= THRESHOLD_TO_MOVE_IN_RECOVERING;
		if (state == "RECOVERING")
		{
			if (Info.self.bulletNum >= THRESHOLD_TO_END_RECOVERING)
			{
				state = "";
				Ex.Clear();
			}
			else
			{
				RecoverBullet.go(Move.M.GetTarget(), can_move_in_recovering);
				return;
			}
		}
		if (state == "")
		{
			if (Info.self.bulletNum <= THRESHOLD_TO_START_RECOVERING)
			{
				state = "RECOVERING";
				RecoverBullet.start();
				RecoverBullet.go(Move.M.GetTarget(), can_move_in_recovering);
				return;
			}
			CheckProp(Move.M.last_targ);
			if (Ex.TimeLeft() < 0)
			{
				if (Info.IsStucked())
				{
					MoveE.Escape();
				}
				else
				{
					int cnt = max(1, Info.self.bulletNum - THRESHOLD_FOR_MAX_BULLET);
					while (cnt--) Shoot.go();
					Move.go();
				}
			}
		}
	}
};


void GlobalInitialize()
{
	int job = GetJobId(playerJob);
	// if (job == 0) BULLET_VELOCITY = Constants::BulletMoveSpeed::bullet0;
	if (job == 1)
	{
		BULLET_VELOCITY = Constants::BulletMoveSpeed::bullet1;
		BULLET_PAINT_R = 1;
		CAN_SHOOT_TO_RECOVER = true;
	}
	// if (job == 2) BULLET_VELOCITY = Constants::BulletMoveSpeed::bullet2;
	if (job == 3)
	{
		BULLET_VELOCITY = Constants::BulletMoveSpeed::bullet3;
		CAN_SHOOT_TO_RECOVER = true;
	}
	if (job == 4)
	{
		BULLET_VELOCITY = Constants::BulletMoveSpeed::bullet4;
		BULLET_PAINT_R = 0;
		CAN_SHOOT_TO_RECOVER = false;
	}
	// if (job == 5) BULLET_VELOCITY = Constants::BulletMoveSpeed::bullet5;
	// if (job == 6) BULLET_VELOCITY = Constants::BulletMoveSpeed::bullet6;

	Assert(MOVE_DEFAULT_TIME_BLOCK > DEFAULT_IDLE_TIME);
	Info.Collect(false);
	Info.cell[ToCell(Info.self.x)][ToCell(Info.self.y)] = SELFBIRTH;
	Info.update_cell(ToCell(Info.self.x), ToCell(Info.self.y), SELFBIRTH, 2e9);
	Info.self_birth_cx = ToCell(Info.self.x);
	Info.self_birth_cy = ToCell(Info.self.y);

	auto IDs = G->GetPlayerGUIDs();
	TEAM_NUM = IDs.size();
	PLAYER_NUM = IDs[0].size();
	for (int i = 0; i < TEAM_NUM; i++) for (int j = 0; j < PLAYER_NUM; j++)
	{
		guid_to_playerid[IDs[i][j]] = make_pair(i, j);
	}
	Assert(guid_to_playerid.find(Info.self.guid) != guid_to_playerid.end());
	TEAM_ID = guid_to_playerid[Info.self.guid].first;
	PLAYER_ID = guid_to_playerid[Info.self.guid].second;
	cout << "TEAM_ID = " << TEAM_ID << " PLAYER_ID = " << PLAYER_ID << endl;

	int spx[8] = { 3, 2, 3, 2, 46, 47, 46, 47 };
	int spy[8] = { 2, 3, 47, 46, 2, 3, 47, 46 };
	for (int i = 0; i < TEAM_NUM; i++) for (int j = 0; j < PLAYER_NUM; j++)
	{
		int id = i * PLAYER_NUM + j;
		guid_to_spawn[IDs[i][j]] = make_pair(spx[id], spy[id]);
		cerr << spx[id] << "," << spy[id] << endl;
	}
}


void Play()
{
	if (play_count == 0)
	{
		GlobalInitialize();
	}
	/*
	if (play_count % 100 == 0)
	{
		cout << "Score = " << G->GetTeamScore() << "  time = " << GetTime() / 1000 << endl;
	}
	if (play_count % 20 == 0)
	{
		cout << "I'm at (" << Info.self.x << "," << Info.self.y << ") state = " << state << " bulletNum = " << Info.self.bulletNum << endl;
		for (auto t : Info.enemy)
		{
			auto e = t.second;
			cerr << "ID = " << guid_to_playerid[e.guid].second << " " << ToCell(e.x) << "," << ToCell(e.y) << " upd_time = " << e.upd_time << " hp=" << e.hp << " hp_time = " << e.hp_time << " ";
			if (e.GetTimeSinceRespawn() > 27 * 1000 && e.GetTimeSinceRespawn() < 35 * 1000)
			{
				cerr << "is about to respawn!!!" << endl;
			}
			cerr << endl;
		}
	}
	*/
	if (play_count % 200 == 0)
	{
		if (!NO_DEBUG_OUT) Info.DebugFullMap();
	}

	int play_start_time = GetTime();
	Info.Collect();
	Info.CheckInbox();
	if (playerJob == THUAI4::JobType::Job1) JOB1::Commander();
	if (playerJob == THUAI4::JobType::Job3) JOB3::Commander();
	if (playerJob == THUAI4::JobType::Job4) JOB4::Commander();
	Ex.Execute();
	while (GetTime() < play_start_time + PLAY_TIME_WINDOW - 2)
	{
		Ex.Execute();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	play_count++;
}

void AI::play(GameApi& g)
{
	G = &g;
	Play();
}
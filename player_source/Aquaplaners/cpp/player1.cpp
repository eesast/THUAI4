#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = false;

#include <random>
#include <iostream>
#include <sstream>
#include <list>
#include <algorithm>
#include <fstream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

#include <chrono>
#include <thread>
//my player id is 3
namespace env {
	bool isAlive = false;
	const int moveTime = 30;
	const double extremeDanger = 1e8;
	const double hugeDanger = 20011230.0;
	const double mediumDanger = 114514.0;
	const double tinyDanger = 1919.810;
	const double ang[] = { 0.000000000000, 0.785398163397, 1.570796326795, 2.356194490192,
		3.141592653590, 3.926990816987, 4.712388980385,5.497787143782, };
	const int block[] = { 0, 1, 0, 1, 2, 2, 1, 1, 1, 0, 2, 1, 0, 2, 0, 1, 2, 1, 0, 2, 0, 0, 0, 1, 2,
		2, 0, 0, 2, 0, 1, 0, 2, 2, 2, 1, 1, 2, 0, 0, 2, 0, 0, 1, 1, 0, 2, 1, 0, 1, 0, 2, 1, 2, 0, 1,
		1, 0, 1, 1, 0, 1, 2, 0, 0, 1, 2, 0, 0, 2, 1, 1, 0, 1, 0, 2, 0, 2, 0, 1, 1, 0, 2, 2, 1, 2, 2,
		1, 2, 1, 0, 1, 2, 2, 1, 1, 1, 0, 1, 2, 1, 2, 2, 0, 1, 1, 0, 2, 2, 1, 0, 0, 2, 0, 1, 2, 1, 0,
		2, 1, 1, 2, 2, 0, 2, 2, 0, 1, 0, 1, 0, 1, 1, 0, 1, 2, 1, 1, 2, 0, 1, 2, 2, 1, 2, 1, 0, 0, 1,
		0, 2, 0, 0, 2, 2, 0, 1, 2, 0, 1, 0, 1, 2, 2, 1, 0, 2, 1, 0, 0, 2, 1, 2, 1, 2, 1, 2, 0, 2, 1,
		1, 2, 2, 1, 0, 1, 1, 2, 1, 1, 2, 2, 0, 2, 2, 2, 0, 0, 1, 0, 0, 2, 2, 0, 1, 0, 2, 2, 1, 0, 1,
		0, 0, 2, 2, 1, 2, 0, 0, 1, 2, 0, 2, 2, 0, 1, 2, 1, 2, 0, 2, 0, 1, 0, 1, 0, 2, 0, 2, 0, 1, 1,
		1, 0, 2, 0, 0, 2, 0, 1, 1, 2, 2, 0, 1, 0, };
	int standX[2][4] = { 16500, 5500, 10500, 5500, 33500, 44500, 40500, 42500 };
	int standY[2][4] = { 8500, 17500, 43500, 33500, 7500, 18500, 43500, 33500 };
	std::shared_ptr<const THUAI4::Character> self;
	const int playerID = 0;
	int callTime = 0;
	long long start;
	int64_t birthpointID = 0;
	int birthpointX = 0, birthpointY = 0;
	THUAI4::ColorType teamColor;
	uint16_t teamID;
	std::uniform_real_distribution<double> randreal(0, 1);
	std::pair<uint32_t, uint32_t> now, destination = { 25000, 25000 };
	THUAI4::ColorType colorArray[7][7];
	std::vector<int> shootList;
	class Ray {
	public:
		std::pair<double, double> point;
		std::pair<double, double> vec;
		Ray(std::pair<double, double> p, std::pair<double, double> v);
		std::vector<double> GetEquation();
	};
	double dist(std::pair<double, double> p, Ray r);
	double dist(std::pair<uint32_t, uint32_t> p1, std::pair<uint32_t, uint32_t> p2);
	template <typename T, typename U> double dist(T x1, T y1, U x2, U y2);
	inline double randomWithAmplitude(double amplitude);
	void Initialize();
	void Locate(GameApi& g);
	void GetColorArray(GameApi& g);
	double BirthpointDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::BirthPoint>>& bp);
	double WallDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::Wall>>& w);
	double PropDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::Prop>>& pr);
	double CellDanger(std::pair<uint32_t, uint32_t>& p);
	double CenterDanger(std::pair<uint32_t, uint32_t>& p);
	double SomewhereDanger(std::pair<uint32_t, uint32_t>& p, std::pair<uint32_t, uint32_t>& des);
	double CharDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::Character>>& ch);
	auto GetTime() {
		return std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	void DodgeBullets(std::vector<std::shared_ptr<const THUAI4::Bullet>>& b,
		std::vector<std::shared_ptr<const THUAI4::Character>>& ch, GameApi& g);
	void SafeMove(GameApi& g);
	namespace msg {
		std::string messageToSend;
		void encode(GameApi& g, THUAI4::Character ch);
		std::pair<uint32_t, uint32_t> player[4];
		struct enemy {
			uint32_t x, y;
			int32_t v, vx, vy, hp;
		};
		std::list<enemy> enemyList;
		int calltime_player[7] = { 0 };
		int playertype;
		bool hasCalled[7] = { false };
	}
	std::pair<uint32_t, uint32_t> BlockXY(int id);
	void Shoot(GameApi& g, uint32_t x, uint32_t y);
	double tempd;
}

env::Ray::Ray(std::pair<double, double> p, std::pair<double, double> v) : point(p), vec(v) {

}

std::vector<double> env::Ray::GetEquation() {
	std::vector<double> res = { vec.second, -vec.first, point.second * vec.first - point.first * vec.second };
	return res;
}

double env::dist(std::pair<double, double> p, env::Ray r) {
	std::vector<double> eq = r.GetEquation();
	if ((p.first - r.point.first) * r.vec.first + (p.second - r.point.second) * r.vec.second < 0) {
		return -dist(p, r.point);
	}
	else {
		return fabs(eq[0] * p.first + eq[1] * p.second + eq[2]) / sqrt(eq[0] * eq[0] + eq[1] * eq[1]);
	}
}

double env::dist(std::pair<uint32_t, uint32_t> p1, std::pair<uint32_t, uint32_t> p2) {
	return sqrt(pow(double(p1.first) - double(p2.first), 2) + pow(double(p1.second) - double(p2.second), 2));
}

template <typename T, typename U> double env::dist(T x1, T y1, U x2, U y2) {
	return sqrt(pow(double(x1) - double(x2), 2) + pow(double(y1) - double(y2), 2));
}

inline double env::randomWithAmplitude(double amplitude) {
	return 2 * amplitude * randreal(::e) - amplitude;
}

void env::Initialize() {
	isAlive = true;
	destination.first = standX[teamID][playerID];
	destination.second = standY[teamID][playerID];
	for (int i = 0; i < 7; i++) env::msg::hasCalled[i] = false;
	for (int i = 0; i < 7; i++) env::msg::calltime_player[i] = callTime;
}

void env::Locate(GameApi& g) {
	now.first = g.GetSelfInfo()->x;
	now.second = g.GetSelfInfo()->y;
}

void env::GetColorArray(GameApi& g) {
	for (int i = Constants::Map::GridToCellX(now) - 3, ii = 0; i < Constants::Map::GridToCellX(now) + 3; i++, ii++) {
		for (int j = Constants::Map::GridToCellY(now) - 3, jj = 0; j < Constants::Map::GridToCellY(now) + 3; j++, jj++) {
			if ((i < 0 || i > 49) || (j < 0 || j > 49)) {
				colorArray[ii][jj] = THUAI4::ColorType::Invisible;
			}
			else {
				colorArray[ii][jj] = g.GetCellColor(i, j);
			}
		}
	}
}

double env::BirthpointDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::BirthPoint>>& bp) {
	double s = 0;
	for (auto it = bp.begin(); it != bp.end(); it++) {
		if ((tempd = dist(p.first, p.second, double((*it)->x), double((*it)->y)) < 2500)) {
			s += extremeDanger * exp(-tempd / 2500);
		}
	}
	return s;
}

double env::WallDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::Wall>>& w) {
	double s = 0;
	for (auto it = w.begin(); it != w.end(); it++) {
		if ((tempd = dist(p.first, p.second, (*it)->x, (*it)->y)) < 1500) {
			s += mediumDanger * exp(-tempd / 500);
		}
	}
	return s;
}

double env::PropDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::Prop>>& pr) {
	double s = 0;
	for (auto it = pr.begin(); it != pr.end(); it++) {
		if ((tempd = dist(p.first, p.second, (*it)->x, (*it)->y)) < 1000) {
			s -= mediumDanger * exp(-tempd / 700);
		}
	}
	return s;
}

double env::CellDanger(std::pair<uint32_t, uint32_t>& p) {
	double s = 0;
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 7; j++) {
			if (((p.first / Constants::Map::numOfGridPerCell + i - 3 != birthpointX / Constants::Map::numOfGridPerCell)
				|| (p.second / Constants::Map::numOfGridPerCell + j - 3 != birthpointY / Constants::Map::numOfGridPerCell))
				&& colorArray[i][j] == teamColor) {
				/*s -= tinyDanger * exp(-dist(Constants::Map::CellToGrid(p.first / Constants::Map::numOfGridPerCell + i - 3,
					p.second / Constants::Map::numOfGridPerCell + j - 3), p) / 2000);*/
				s -= tinyDanger;
				if (i == 3 && j == 3) {
					s -= mediumDanger;
				}
			}
		}
	}
	return s;
}

double env::CenterDanger(std::pair<uint32_t, uint32_t>& p) {
	return -tinyDanger * exp(-dist(p, Constants::Map::CellToGrid(25, 25)) / 30000);
}

double env::SomewhereDanger(std::pair<uint32_t, uint32_t>& p, std::pair<uint32_t, uint32_t>& des) {
	return -2 * mediumDanger * exp(-dist(p, des) / 30000);
}

double env::CharDanger(std::pair<uint32_t, uint32_t>& p, std::vector<std::shared_ptr<const THUAI4::Character>>& ch) {
	double s = 0;
	for (auto it = ch.begin(); it != ch.end(); it++) {
		if ((*it)->guid!=self->guid && (tempd = dist(p.first, p.second, (*it)->x, (*it)->y)) < 1500)
			s += hugeDanger * exp(-tempd);
	}
	return s;
}

void env::DodgeBullets(std::vector<std::shared_ptr<const THUAI4::Bullet>>& b,
	std::vector<std::shared_ptr<const THUAI4::Character>>& ch, GameApi& g) {
	auto speed = g.GetSelfInfo()->moveSpeed;
	auto bp = g.GetBirthPoints();
	auto w = g.GetWalls();
	double minDanger = 1e+100;
	int minDir = 0;
	for (int dir = 0; dir < 8; dir++) {
		uint32_t targetX = now.first + double(speed * cos(ang[dir]) * moveTime) / 1000,
			targetY = now.second + double(speed * sin(ang[dir]) * moveTime) / 1000;
		targetX = ((targetX < 0) ? 0 : targetX) > 50000 ? 50000 : targetX;
		targetY = ((targetY < 0) ? 0 : targetY) > 50000 ? 50000 : targetY;
		double sumDanger = 0;
		auto p = std::make_pair(targetX, targetY);
		for (auto it = b.begin(); it != b.end(); it++) {
			if ((*it)->teamID != teamID) {
				double d = dist(std::make_pair(targetX, targetY), Ray(std::make_pair((*it)->x, (*it)->y),
					std::make_pair(cos((*it)->facingDirection), sin((*it)->facingDirection))));
				if (d < 0) {
					if (int((*it)->radius) + Constants::Map::numOfGridPerCell + d > 0) {
						sumDanger += hugeDanger * exp(d / (*it)->radius);
					}
					else {
						continue;
					}
				}
				else {
					if (int((*it)->radius) + Constants::Map::numOfGridPerCell > d) {
						sumDanger += hugeDanger * exp(-d / (*it)->radius);
					}
					else {
						continue;
					}
				}
			}
		}
		for (auto it = ch.begin(); it != ch.end(); it++) {
			if ((*it)->teamID != teamID) {
				sumDanger += 2 * hugeDanger * exp(-dist(targetX, targetY, (*it)->x, (*it)->y));
			}
		}
		sumDanger += BirthpointDanger(p, bp);
		sumDanger += WallDanger(p, w);
		sumDanger += CharDanger(p, ch);
		if (sumDanger < minDanger) {
			minDanger = sumDanger;
			minDir = dir;
		}
	}
	g.MovePlayer(env::moveTime, ang[minDir]);
	std::this_thread::sleep_for(std::chrono::milliseconds(env::moveTime));
}

void env::SafeMove(GameApi& g) {
	auto speed = g.GetSelfInfo()->moveSpeed;
	auto bp = g.GetBirthPoints();
	auto w = g.GetWalls();
	auto pr = g.GetProps();
	auto ch = g.GetCharacters();
	double minDanger = 1e+100;
	int minDir = 0;
	long long ms = GetTime();
	for (int dir = 0; dir < 8; dir++) {
		uint32_t targetX = now.first + double(speed * cos(ang[dir]) * moveTime / 1000),
			targetY = now.second + double(speed * sin(ang[dir]) * moveTime / 1000);
		targetX = ((targetX < 0) ? 0 : targetX) > 50000 ? 50000 : targetX;
		targetY = ((targetY < 0) ? 0 : targetY) > 50000 ? 50000 : targetY;
		auto p = std::make_pair(targetX, targetY);
		double sumDanger = 0;
		sumDanger += BirthpointDanger(p, bp);
		sumDanger += WallDanger(p, w);
		sumDanger += PropDanger(p, pr);
		//sumDanger += CenterDanger(p);
		//auto des = Constants::Map::CellToGrid(40, 15);
		sumDanger += CharDanger(p, ch);
		sumDanger += 5 * SomewhereDanger(p, destination);
		if (sumDanger < minDanger) {
			minDanger = sumDanger;
			minDir = dir;
		}
	}
	g.MovePlayer(env::moveTime, ang[minDir] + randomWithAmplitude(0.2));
	std::this_thread::sleep_for(std::chrono::milliseconds(env::moveTime));
}

void env::msg::encode(GameApi& g, THUAI4::Character ch) {
	std::stringstream ss("");
	ss << ch.x << " " << ch.y << " " << ch.moveSpeed * ch.isMoving << " " << int(ch.moveSpeed * ch.isMoving * cos(ch.facingDirection))
		<< " " << int(ch.moveSpeed * ch.isMoving * sin(ch.facingDirection)) << " " << ch.hp;
	messageToSend = ss.str();
}


std::pair<uint32_t, uint32_t> env::BlockXY(int id) {
	return Constants::Map::CellToGrid((id / 16) * 3 + 2, (id % 16) * 3 + 2);
}

void env::Shoot(GameApi& g, uint32_t x, uint32_t y) {
	uint32_t t = round(1000 * dist(x, y, now.first, now.second) / Constants::BulletMoveSpeed::bullet1);
	//std::cerr << x << " " << y << std::endl;
	if (x == now.first) {
		if (y > now.second) {
			g.Attack(t, ang[2]);
		}
		else {
			g.Attack(t, ang[6]);
		}
	}
	else {
		if (x < now.first) {
			//std::cerr << atan((double(y) - double(now.second)) / (double(x) - double(now.first))) << std::endl;
			g.Attack(t, atan((double(y) - double(now.second)) / (double(x) - double(now.first))) + ang[4]);
		}
		else {
			g.Attack(t, atan((double(y) - double(now.second)) / (double(x) - double(now.first))));
		}
	}
}

double pole_angle(unsigned int selfx, unsigned int selfy, double x, double y)
{
	return(atan((double)(y - selfy) / (double)(x - selfx)) + 3.1415926 * (selfx > x));
}

double bullet_angle(unsigned int x, unsigned int y, double v_x, double v_y,						//子弹极角
	unsigned int selfx, unsigned int selfy, unsigned int v)
{
	double theta = pole_angle(selfx, selfy, x, y);
	if (v != 0) {
		double phi = pole_angle(0, 0, v_x, v_y);
		return (theta + acos((double)v * cos(phi - theta) / Constants::BulletMoveSpeed::bullet4) - 1.570796);
	}
	else return theta;
}

double boom_time(unsigned int x, unsigned int y, double v_x, double v_y,						//子弹极角
	unsigned int selfx, unsigned int selfy, unsigned int v)										//子弹爆炸时间
{
	double theta = pole_angle(selfx, selfy, x, y);
	if (v != 0) {
		double phi = pole_angle(0, 0, v_x, v_y);
		return(1000 * sqrt((double)((x - selfx) * (x - selfx) + (y - selfy) * (y - selfy))) /
			(Constants::BulletMoveSpeed::bullet1 * sin(acos((double)v * cos(phi - theta) / Constants::BulletMoveSpeed::bullet1)) - v * cos(theta - phi)));
	}
	else return 1000 * sqrt(double((x - selfx) * (x - selfx) + (y - selfy) * (y - selfy)) / Constants::BulletMoveSpeed::bullet1);
}

void selfdefense(GameApi& g)
{
	auto ch = g.GetCharacters();
	for (auto it = ch.begin(); it != ch.end(); it++) {
		if ((*it)->teamID != env::teamID && (*it)->jobType != THUAI4::JobType::Job6) {
			double angle = bullet_angle((*it)->x, (*it)->y, (*it)->moveSpeed * cos((*it)->facingDirection) * (*it)->isMoving / 2,
				(*it)->moveSpeed * sin((*it)->facingDirection) * (*it)->isMoving / 2, g.GetSelfInfo()->x, g.GetSelfInfo()->y,
				(*it)->moveSpeed * (*it)->isMoving);
			double time = boom_time((*it)->x, (*it)->y, (*it)->moveSpeed * cos((*it)->facingDirection) * (*it)->isMoving / 2,
				(*it)->moveSpeed * sin((*it)->facingDirection) * (*it)->isMoving / 2, g.GetSelfInfo()->x, g.GetSelfInfo()->y,
				(*it)->moveSpeed * (*it)->isMoving);
			if (env::self->bulletNum > 2) g.Attack(time, angle);
			//std::this_thread::sleep_for(std::chrono::milliseconds(env::moveTime));
		}
	}
}

void AI::play(GameApi& g) {
	//freopen("log.txt", "w+", stdout);
	env::self = g.GetSelfInfo();
	bool alive = !env::self->isDying;
	if (alive) {
		if (!env::isAlive) {
			env::Initialize();
		}
		auto bullet = g.GetBullets();
		env::start = env::GetTime();
		if (!env::callTime++) {
			//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
			env::teamColor = g.GetSelfTeamColor();
			env::teamID = env::self->teamID;
			env::start = env::GetTime();
			auto bpv = g.GetBirthPoints();
			for (auto it = bpv.begin(); it != bpv.end(); it++) {
				if ((*it)->x == env::self->x && (*it)->y == env::self->y) {
					env::birthpointID = (*it)->guid;
					env::birthpointX = Constants::Map::GridToCellX(std::make_pair((*it)->x, (*it)->y));
					env::birthpointY = Constants::Map::GridToCellY(std::make_pair((*it)->x, (*it)->y));
					break;
				}
			}
		}
		env::Locate(g);
		auto b = g.GetBullets();
		auto ch = g.GetCharacters();
		bool safe = true;
		for (auto it = b.begin(); it != b.end() && safe; it++) {
			if ((*it)->teamID != env::teamID) {
				safe = false;
			}
		}
		for (auto it = ch.begin(); it != ch.end() && safe; it++) {
			if ((*it)->teamID != env::teamID) {
				safe = false;
			}
		}
		if (!safe) {
			for (auto it = ch.begin(); it != ch.end(); it++) {
				if ((*it)->teamID != env::teamID) {
					switch ((*it)->jobType)
					{
					case THUAI4::JobType::Job0: env::msg::playertype = 0; break;
					case THUAI4::JobType::Job1: env::msg::playertype = 1; break;
					case THUAI4::JobType::Job2: env::msg::playertype = 2; break;
					case THUAI4::JobType::Job3: env::msg::playertype = 3; break;
					case THUAI4::JobType::Job4: env::msg::playertype = 4; break;
					case THUAI4::JobType::Job5: env::msg::playertype = 5; break;
					case THUAI4::JobType::Job6: env::msg::playertype = 6; break;
					}
					if (env::msg::hasCalled[env::msg::playertype])
					{
						if (env::callTime - env::msg::calltime_player[env::msg::playertype] > 100)
							env::msg::hasCalled[env::msg::playertype] = false;
					}
					if (env::msg::playertype != 6 && !env::msg::hasCalled[env::msg::playertype])
					{
						env::msg::calltime_player[env::msg::playertype] = env::callTime;
						env::msg::hasCalled[env::msg::playertype] = true;
						env::msg::encode(g, **it);
						g.Send(3, env::msg::messageToSend);
					}
					if (env::self->bulletNum > 2) {
						selfdefense(g);
					}
				}
			}
			env::DodgeBullets(bullet, ch, g);
			env::Locate(g);
			env::destination = env::now;
		}
		else {
			auto pr = g.GetProps();
			for (auto it = pr.begin(); it != pr.end(); it++) {
				if (Constants::Map::GridToCellX(std::make_pair((*it)->x, (*it)->y)) == env::now.first
					&& Constants::Map::GridToCellY(std::make_pair((*it)->x, (*it)->y)) == env::now.second) {
					g.Pick((*it)->propType);
				}
			}
			if (env::now.first / Constants::Map::numOfGridPerCell == env::destination.first / Constants::Map::numOfGridPerCell
				&& env::now.second / Constants::Map::numOfGridPerCell == env::destination.second / Constants::Map::numOfGridPerCell) {
				if (g.GetCellColor(env::now.first / Constants::Map::numOfGridPerCell, env::now.second / Constants::Map::numOfGridPerCell) != env::teamColor) {
					if (env::self->bulletNum) {
						g.Attack(0, 0);
					}
				}
			}
			else {
				env::SafeMove(g);
			}
			if (env::shootList.empty()) {
				for (int i = 0; i < 256; i++) {
					if (env::block[i] == env::playerID) {
						env::shootList.push_back(i);
					}
				}
				std::shuffle(env::shootList.begin(), env::shootList.end(), ::e);
			}
			if (env::GetTime() - env::start < 592000) {
				if (env::self->bulletNum > 5) {
					env::Shoot(g, env::BlockXY(env::shootList.back()).first, env::BlockXY(env::shootList.back()).second);
					env::shootList.pop_back();
				}
			}
			else {
				if (env::self->bulletNum) {
					env::Shoot(g, env::BlockXY(env::shootList.back()).first, env::BlockXY(env::shootList.back()).second);
					env::shootList.pop_back();
				}
			}
		}
	}
	else {
		env::isAlive = false;
	}
}

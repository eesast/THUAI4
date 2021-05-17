#include "AI.h"
#include "Constants.h"

//峈樑寀play()覃蚚潔蚔牁袨怓載陔郯ㄛ峈淩寀硐悵痐絞蚔牁袨怓祥頗掩袨怓載陔滲杅迵GameApi腔源楊肮奀溼恀
extern const bool asynchronous = true;

#include <random>
#include <iostream>

/* 衾 VS2019 砐醴扽俶笢羲 C++17 梓袧ㄩ/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job5; //恁忒眥珛ㄛ恁忒 !!斛剕!! 隅砱森曹講懂恁寁眥珛

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}



#include <set>
#include <cstring>
#include <queue>
#include <chrono>
#include <algorithm>

using namespace std;

#define int long long
#define belongcell(belongcell__x) ((int)(belongcell__x/1000))
#define d_belongcell(belongcell__x) ((int)(belongcell__x/1000.00))
#define abs(abs__x) (((abs__x)>0)?(abs__x):(-abs__x))

const int start_time = 1618993950538ll;

inline int getTime() {
	return (1ll * std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count()) - start_time;
}

const double pi = 3.1415926;
const int Massage_compress_time = 256;
const int Massage_compress_multiply[4] = { 1, 256, 256 * 256, 256 * 256 * 256 };
struct enemy_returning_node {
	int x, y; //菩腔釴梓
	int job; //菩腔眥珛
	int speed; //菩厒僅腔湮苤
	double facingDirection; //菩厒僅腔源砃
	bool operator < (const enemy_returning_node& u) const { //笭婥堍呾睫眕晞set赻雄齬唗
		if (x != u.x)
			return x < u.x;
		return y < u.y;
	}
};
queue<shared_ptr<const THUAI4::Wall>> walls_info;
class Massage_Node {
public:
	char str[64];
private:
	void base_write(int first_pos, int length, int val); //輛秶蛌遙綴ㄛ蔚val迡[first_pos,first_pos+length-1]笢
	void base_read(int first_pos, int length, int& val);//黍[first_pos,first_pos+length-1]腔揹ㄛ蛌趙峈坋輛秶綴迡val
	int base_read(int first_pos, int length); //黍[first_pos,first_pos+length-1]腔揹ㄛ蛌趙峈坋輛秶綴迡val
	void update_myinfo(shared_ptr<const THUAI4::Character> myinfo);
	void update_enemies(vector<shared_ptr<const THUAI4::Character>>* enemies, shared_ptr<const THUAI4::Character> myinfo);
	void update_time();
	void update_walls();
public:
	void update_all(shared_ptr<const THUAI4::Character> myinfo
		, vector<shared_ptr<const THUAI4::Character>>* enemies); //載陔堤籵捅測鎢ㄐ
	Massage_Node() {
		//str.clear();
		memset(str, 0, sizeof str);
	}
	void sendMassage(GameApi* api);
	void read_mate_job(int& mate_job); //剒猁袧掘殿隙硉腔揣湔弇离
	int read_mate_job(); //眻諉殿隙勦衭眥珛
	void read_mate_location(int& matex, int& matey); //剒猁袧掘謗跺殿隙硉腔揣湔弇离
	pair<int, int> read_mate_location(); //眻諉殿隙珨跺釴梓
	void read_enemies(set<enemy_returning_node>* enemies); //袧掘珨跺set<enemy_returning_node>ㄛ綴蟀覂黍垀衄勦衭換懂陓洘腔enemiesㄛ眕晞笭﹝
	void read_walls(pair<int, int>* walls); //袧掘珨跺pair杅郪睿郪囀侐跺褫葡裔啋匼[a,a+1,a+2,a+3]ㄛ覃蚚read_walls(walls+a)﹝
	void read_walls(vector<pair<int, int>>* walls); //袧掘珨跺pair郪傖腔vectorㄛ陔腔蔚掩隍婓綴醱﹝
	bool getMassage(GameApi* api);
};
void Massage_Node::base_write(int first_pos, int length, int val) { //輛秶蛌遙綴ㄛ蔚val迡[first_pos,first_pos+length-1]笢
	for (int i = 0; i < length; i++)
		str[first_pos + length - i - 1] = (char)((unsigned char)
			((val / Massage_compress_multiply[i]) % Massage_compress_time));
}
void Massage_Node::base_read(int first_pos, int length, int& val) { //黍[first_pos,first_pos+length-1]腔揹ㄛ蛌趙峈坋輛秶綴迡val
	val = 0;
	for (int i = 0; i < length; i++)
		val += str[first_pos + length - i - 1] * Massage_compress_multiply[i];
}
int Massage_Node::base_read(int first_pos, int length) { //黍[first_pos,first_pos+length-1]腔揹ㄛ蛌趙峈坋輛秶綴迡val
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
		base_write(9 * i + 7, 2, (*ic)->x - myinfo->x + 10000); //蚚萃縐嫌釴梓桶尨眈勤弇离
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
	//剒猁袧掘殿隙硉腔揣湔弇离
	mate_job = base_read(0, 1);
}
int Massage_Node::read_mate_job() { //眻諉殿隙勦衭眥珛
	return base_read(0, 1);
}
void Massage_Node::read_mate_location(int& matex, int& matey) {
	//剒猁袧掘謗跺殿隙硉腔揣湔弇离
	matex = base_read(1, 3);
	matey = base_read(4, 3);
}
pair<int, int> Massage_Node::read_mate_location() { //眻諉殿隙珨跺釴梓
	return pair<int, int>(base_read(1, 3), base_read(4, 3));
}
void Massage_Node::read_enemies(set<enemy_returning_node>* enemies) {
	//袧掘珨跺set<enemy_returning_node>ㄛ綴蟀覂黍垀衄勦衭換懂陓洘腔enemiesㄛ眕晞笭﹝
	enemy_returning_node tmp;
	int matex, matey;
	read_mate_location(matex, matey);
	for (int i = 0; i < 4; i++) { //載陔婦漪腔侐跺菩
		tmp.x = base_read(9 * i + 7, 2) - 10000 + matex;
		tmp.y = base_read(9 * i + 9, 2) - 10000 + matey;
		tmp.job = base_read(9 * i + 11, 1);
		if (tmp.x == 0 && tmp.y == 0 && tmp.job == 0) //載陔俇賸
			break;
		tmp.facingDirection = ((double)(base_read(9 * i + 12, 2))) * 2 * pi / 180.0 / 180.0;
		tmp.speed = base_read(9 * i + 14, 2);
		enemies->insert(tmp);
	}
}
void Massage_Node::read_walls(pair<int, int>* walls) {
	//袧掘珨跺pair杅郪睿郪囀侐跺褫葡裔啋匼[a,a+1,a+2,a+3]ㄛ覃蚚read_walls(walls+a)﹝
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
	//赻雄徹薦裁赻撩楷堤腔陓洘ㄐ
	shared_ptr<const THUAI4::Character> myinfo = api->GetSelfInfo();
	if (base_read(0, 1) == ((int)((unsigned char)(myinfo->jobType)))) { //籵徹瓚剿眥珛岆瘁眈肮懂徹薦赻撩腔陓洘
		if (!api->MessageAvailable())
			return false;
		api->TryGetMessage(st);
		st.copy(str, 64, 0);
	}
	//彆祥洷咡徹薦裁赻撩腔陓洘ㄛ刉壺涴謗沭蛁庋潔腔測鎢﹝
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
	enemy_node() :val(1) {}
	enemy_node(shared_ptr<const THUAI4::Character> __data, double __val) : data(__data), val(__val) {}
	bool operator < (const enemy_node& u) const {
		return val < u.val;
	}
};

inline double getValue(shared_ptr<const THUAI4::Character> x, shared_ptr<const THUAI4::Character> myinfo, double distance) {
	//蚥珂脹撰瓚剿ㄛ隅扂蠅腔馴僻醴梓

	//褫眕眻諉湖
	double available = 1.0;
	if (abs(belongcell(x->y) - belongcell(myinfo->y)) == 2 && abs(belongcell(x->x) - belongcell(myinfo->x)) <= 2)
		available = 100000000000.00; //眻諉憩褫眕韜笢ㄐ砩祥砩俋ㄐ
	if (myinfo->bulletNum == 0)
		available = -1; //挎挎睎羶赽粟賸ㄛ枅韜猁踡ㄐ

	double jobval;
	switch ((int)((unsigned char)(x->jobType))) {
	case 0: case 5: jobval = 1; //褫眕蕉藉輛馴
		break;
	case 1: case 4: jobval = 4; //竭褫眕輛馴
		break;
	case 3: jobval = 20; //苤豜赶ˋ伀ㄐ
		break;
	case 2: jobval = 0.01; //撓綱祥猁輛馴
		break;
	case 6: jobval = 0;//湖侚祥猁輛馴
	}

	double distval = 1000.0 / (1.0 * distance);

	double speedval;
	if (x->moveSpeed > myinfo->moveSpeed)
		speedval = -1; //袚祥奻ㄛ頗掩袚ㄐ變變變ㄐ
	else if (x->moveSpeed == myinfo->moveSpeed)
		speedval = 0; //羶衄厒僅蚥岊ㄛ呾賸呾賸
	else {
		if (x->moveSpeed <= myinfo->moveSpeed / 2)
			speedval = 100; //褫眕陛ㄐ
		if (x->moveSpeed < myinfo->moveSpeed / 4)
			speedval = 1000; //鏝婭ㄐ
	}

	if (available < 0 || speedval < 0)
		return -1;
	return available * speedval * distval * jobval;
}
int knowntime[55][55]; //郔綴珨棒掩潼弝徹腔奀潔ㄛ蚔軗源砃瓚隅奀妏蚚
bool is_known[55][55];
int knownmap[55][55]; //帤抻隴-(-1)ㄛ婦漪-32ㄛ俇諾華-(color)

double dist[55][55]; //getdist妏蚚ㄛ悵湔迵弝珧囀誹萸潔腔擒燭
double dist_without_restriction[55][55];
void getdist(pair<int, int> cur, bool first_run = true) {
	if (first_run)
		memset(dist, 0x3f, sizeof(dist)),
		dist[cur.first][cur.second] = 0;
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			if (abs(i) + abs(j) == 1 &&
				knownmap[cur.first + i][cur.second + j] != 32 &&
				dist[cur.first + i][cur.second + j] > dist[cur.first][cur.second] + 1.0 &&
				is_known[cur.first + i][cur.second + j])
				dist[cur.first][cur.second] = dist[cur.first][cur.second] + 1.0,
				getdist(pair<int, int>(cur.first + i, cur.second + j), false);
}
void getdist_without_restriction(pair<int, int> cur, bool first_run = true) {
	if (first_run)
		memset(dist_without_restriction, 0x3f, sizeof(dist_without_restriction)),
		dist_without_restriction[cur.first][cur.second] = 0;
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			if (abs(i) + abs(j) == 1 &&
				knownmap[cur.first + i][cur.second + j] != 32 && knownmap[cur.first + i][cur.second + j] != -1 &&
				dist_without_restriction[cur.first + i][cur.second + j] > dist_without_restriction[cur.first][cur.second] + 1.0)
				dist_without_restriction[cur.first][cur.second] = dist_without_restriction[cur.first][cur.second] + 1.0,
				getdist_without_restriction(pair<int, int>(cur.first + i, cur.second + j), false);
}
double supplement(shared_ptr<const THUAI4::Character> myinfo) {
	int bullet_potential = myinfo->maxBulletNum - myinfo->bulletNum;
	getdist_without_restriction(pair<int, int>(belongcell(myinfo->x), belongcell(myinfo->y)));
	double mn = 20000000.00;
	for (int i = 0; i < 50; i++)
		for (int j = 0; j < 50; j++)
			if (knownmap[i][j] == myinfo->teamID && mn > dist_without_restriction[i][j])
				mn = dist_without_restriction[i][j];
	return 1.0 * bullet_potential / mn;
}
bool cmp_deltdir(pair<double, int> x, pair<double, int> y) {
	return x.first > y.first;
}
double findroute(int destx, int desty, int x, int y, double qdis) {
	if (x == belongcell(destx) && y == belongcell(desty))
		return 0;
	if (qdis <= -500)
		return 19260817;
	double last;
	if ((last = findroute(destx, desty, x - 1, y, qdis - 1)) != 19260817)
		return pi;
	if ((last = findroute(destx, desty, x + 1, y, qdis - 1)) != 19260817)
		return 0;
	if ((last = findroute(destx, desty, x, y - 1, qdis - 1)) != 19260817)
		return pi * 3.0 / 2.0;
	if ((last = findroute(destx, desty, x - 1, y, qdis - 1)) != 19260817)
		return pi / 2.0;
	return 19260817;
}
void AI::play(GameApi& g) { //嗚赽粟&蚔軗&惆豢醴梓
//黍甜揣湔杅擂
	THUAI4::ColorType mycolor = g.GetSelfTeamColor();
	shared_ptr<const THUAI4::Character> myinfo = g.GetSelfInfo();
	vector<shared_ptr<const THUAI4::Character>> characters = g.GetCharacters();
	vector<shared_ptr<const THUAI4::Wall>> walls = g.GetWalls();
	vector<shared_ptr<const THUAI4::Bullet>> bullets = g.GetBullets();

	static bool firstrun;

	if (firstrun == false) { //忑棒載陔
		for (int i = 0; i < 50; i++)
			for (int j = 0; j < 50; j++)
				knownmap[i][j] = -1;
		memset(knowntime, 0x4f, sizeof(knowntime));
		firstrun = true;
		return;
	}

	vector<shared_ptr<const THUAI4::Wall>>::iterator iw;
	vector<shared_ptr<const THUAI4::Character>>::iterator ic;
	vector<shared_ptr<const THUAI4::Bullet>>::iterator ib;

	for (iw = walls.begin(); iw != walls.end(); iw++) {
		if (knownmap[belongcell((*iw)->x)][belongcell((*iw)->y)] != 32) {
			knownmap[belongcell((*iw)->x)][belongcell((*iw)->y)] = 32;
			walls_info.push(*iw);
		}
	}
	memset(is_known, 0, sizeof(is_known));
	for (int i = 0, tttt = getTime(); i < 50; i++)
		for (int j = 0; j < 50; j++)
			if (cell_visible(i, j, myinfo)) {
				if (knownmap[i][j] != 32)
					knownmap[i][j] = ((unsigned char)(g.GetCellColor(i, j)));
				knowntime[i][j] = tttt;
				is_known[i][j] = true;
			}

	Massage_Node massages;
	massages.update_all(myinfo, &characters);
	massages.sendMassage(&g);

	vector<pair<int, int>> new_walls;
	set<enemy_returning_node> enemies;
	set<enemy_returning_node>::iterator is;
	enemy_returning_node tmp_enemy;
	for (ic = characters.begin(); ic != characters.end(); ic++) {
		if ((*ic)->teamID == myinfo->teamID) //岆勦衭ㄛ祥奪
			continue;
		tmp_enemy.x = (*ic)->x;
		tmp_enemy.y = (*ic)->y;
		tmp_enemy.speed = (*ic)->moveSpeed;
		tmp_enemy.facingDirection = (*ic)->facingDirection;
		tmp_enemy.job = ((int)((unsigned char)(*ic)->jobType));
		enemies.insert(tmp_enemy);
	}
	while (massages.getMassage(&g)) {
		massages.read_enemies(&enemies);
		massages.read_walls(&new_walls);
	}


	//習謹窒煦

#define decorate(x) ((x)<0.0?(2*pi-(x)):(x))



	memset(dist, 0x3f, sizeof(dist));

	getdist(pair<int, int>(belongcell(myinfo->x), belongcell(myinfo->y)));
	enemy_node sorted[10];
	int cnt_sorted = 0;


	for (ic = characters.begin(); ic != characters.end(); ic++) {
		if ((*ic)->teamID == myinfo->teamID) //珆祥夔馴僻勦衭
			continue;
		sorted[++cnt_sorted] = enemy_node(*ic, getValue((*ic), myinfo, dist[belongcell((*ic)->x)][belongcell((*ic)->y)]));
	}
	sort(sorted + 1, sorted + cnt_sorted + 1);


	if (sorted[cnt_sorted].val >= 10000000) { //羲蘿ㄐ
		g.Attack(0, decorate(atan((0.0 + sorted[cnt_sorted].data->y - myinfo->y) / (0.0 + sorted[cnt_sorted].data->x - myinfo->x))));
	}
	else if (sorted[1].val < 0) { //變ㄐ
		double dir[5]; double tmptmp; int cntdir = 1;
		while ((tmptmp = sorted[cntdir].val) < 0.0) {
			dir[cntdir] = decorate(atan((0.0 + sorted[1].data->y - myinfo->y) / (0.0 + sorted[1].data->x - myinfo->x)));
			cntdir++;
		}
		cntdir--;
		sort(dir + 1, dir + cntdir + 1);
		dir[0] = dir[cntdir] - (2 * pi);
		pair<double, int> deltdir[5];
		for (int i = 1; i <= cntdir; i++)
			deltdir[i] = pair<double, int>(dir[i], i);
		sort(deltdir + 1, deltdir + cntdir + 1, cmp_deltdir);
		for (int i = 1; i <= cntdir; i++) {
			dir[i] = decorate(dir[i] - (deltdir[i].first / 2.0));
			if (knownmap[d_belongcell(myinfo->x + cos(dir[i]) * 1000.0)][d_belongcell(myinfo->y + sin(dir[i]) * 1000.0)] == 32)
				continue;
			if (knownmap[d_belongcell(myinfo->x + cos(dir[i]) * 2000.0)][d_belongcell(myinfo->y + sin(dir[i]) * 2000.0)] == 32)
				continue;
			//彆頗癲善赽粟ㄛ崝樓珨跺苤講腔雄
			//隙芛衄諾婬迡勘
			g.MovePlayer(50, dir[i] + (rand() % 200 - 100) * 0.000628);
		}
	}
	else if (sorted[cnt_sorted].val > supplement(myinfo)) { //喳ㄐ
		g.MovePlayer(50, findroute(sorted[cnt_sorted].data->x, sorted[cnt_sorted].data->y, myinfo->x, myinfo->y, dist[sorted[cnt_sorted].data->x][sorted[cnt_sorted].data->y]) + (rand() % 200 - 100) * 0.000628);
	}
	else { //蚔軗 
		double mn = 21474836470000ll; int mnx, mny;
		if (knownmap[belongcell(myinfo->x)][belongcell(myinfo->y)] == myinfo->teamID) {
			g.MovePlayer(50, direction(e));
			return;
		}
		for (int i = 0; i < 50; i++)
			for (int j = 0; j < 50; j++)
				if (knownmap[i][j] == myinfo->teamID && mn > dist_without_restriction[i][j])
					mn = dist_without_restriction[i][j], mnx = i, mny = j;
		if (mn = 21474836470000ll) {
			g.Attack(0, direction(e));
			return;
		}
		g.MovePlayer(50, findroute(mnx, mny, myinfo->x, myinfo->y, dist_without_restriction[mnx][mny]));
	}
} 
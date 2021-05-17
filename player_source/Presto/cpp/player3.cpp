#include "AI.h"
#include "Constants.h"
#include <iomanip>
#include <thread>
#define pi 3.1415926535
#pragma warning(disable:26451)
//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;
#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::MonkeyDoctor; //选手职业，选手 !!必须!! 定义此变量来选择职业
namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * pi);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
std::string space(int k)
{
	std::string s;
	for (int i = 0; i < k; i++)
		s = s + " ";
	return s;
}
std::string infobox(int x, int y, bool isDying, int typeofmessage)
{
	std::string sx = std::to_string(x);
	std::string sy = std::to_string(y);
	std::string sisDying = std::to_string(isDying);
	std::string stypeofmessage = std::to_string(typeofmessage);
	std::string buffer = sx + space(8 - sx.size()) + sy + space(8 - sy.size()) + sisDying + space(1) + stypeofmessage + space(1);//注意长度限制
	return buffer;
}//写入信息，末尾的信息类型计划是用来辨别信息是关于敌还是友的，但若如此需再加一条信息，即队友的编号

struct MessageGroup
{
	int x;
	int y;
	bool isDying;
	int typeofmessage;
};
MessageGroup extractinfo(std::string buffer)
{
	char* cbuffer = new char[buffer.size()];
	cbuffer = buffer.data();
	int ox = atoi(&cbuffer[0]);
	int oy = atoi(&cbuffer[8]);
	bool oisDying = atoi(&cbuffer[16]);
	int otypeofmessage = atoi(&cbuffer[18]);
	MessageGroup r = { ox,oy,oisDying,otypeofmessage };
	return r;
}

static int x00, y00;//出生点坐标
static int calledcount = 0;//调用次数
static int myid;//我方teamID
static int enmid;//敌方teamID
static THUAI4::ColorType mycolor;//我方颜色
static int indanger;//是否被瞄准
static int seekingprp;//是否正在寻找道具
static int beingchased;//是否遇见敌人
static int blocked;//是否卡住
static int restingflag;//是否停下休息，即是否在补充弹药

int distance(uint32_t x, uint32_t y, GameApi& g) {//坐标(x,y)与我间距
	return (int)sqrt(((int)x - (int)g.GetSelfInfo()->x) * ((int)x - (int)g.GetSelfInfo()->x) + ((int)y - (int)g.GetSelfInfo()->y) * ((int)y - (int)g.GetSelfInfo()->y));
}
double angle(uint32_t x, uint32_t y, GameApi& g) {//我到坐标(x,y)的角度
	return atan2((int)y - (int)g.GetSelfInfo()->y, (int)x - (int)g.GetSelfInfo()->x);
}

void AI::play(GameApi& g)
{
	calledcount++;
	auto guid = g.GetPlayerGUIDs();

	if (calledcount == 1)
	{//初始化,从出生点走出第一步并开第一枪,只有第一次调用时会执行
		myid = g.GetSelfInfo()->teamID;
		enmid = 1 ^ myid;
		mycolor = g.GetSelfTeamColor();
		x00 = (int)g.GetSelfInfo()->x;
		y00 = (int)g.GetSelfInfo()->y;
		if (myid == 0) {
			g.MoveDown(200);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			g.Attack(0, 0);
		}
		else {
			g.MoveUp(200);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			g.Attack(0, pi);
		}
	}

	int my_x = int(g.GetSelfInfo()->x);
	int my_y = int(g.GetSelfInfo()->y);
	auto a = g.GetBullets();
	while (a.size() != 0)
	{
		for (int i = 0; i < a.size(); i++)
		{
			if (a[i]->teamID != myid)
			{

				double dis2 = abs((my_y - int(a[i]->y)) - tan(a[i]->facingDirection) * (my_x - int(a[i]->x))) / sqrt(1 + tan(a[i]->facingDirection) * tan(a[i]->facingDirection));
				{ if (dis2 <= 1250)//这个条件视情况可以忽略，忽略之后将增加不必要的走动。但同时如果对方利用沾染范围故意偏离你打，就可以躲开（不删则躲不开）
				{
					if ((abs((int(my_y) + sin(1.57 + a[i]->facingDirection) * 100 - int(a[i]->y)) - tan(a[i]->facingDirection) * (int(my_x) + cos(1.57 + a[i]->facingDirection) * 100 - int(a[i]->x))) - abs(int(my_y - a[i]->y) - tan(a[i]->facingDirection) * int(my_x - a[i]->x))) >= 0)
					{
						g.MovePlayer(650, 1.57 + a[i]->facingDirection); my_x = int(g.GetSelfInfo()->x); my_y = int(g.GetSelfInfo()->y);
						std::this_thread::sleep_for(std::chrono::milliseconds(650));
					}
					else
					{
						g.MovePlayer(650, -1.57 + a[i]->facingDirection); my_x = int(g.GetSelfInfo()->x); my_y = int(g.GetSelfInfo()->y);
						std::this_thread::sleep_for(std::chrono::milliseconds(650));
					}
				}
				}
			}
		}
		g.Wait();
		a = g.GetBullets();
	}
	my_x = int(g.GetSelfInfo()->x);
	my_y = int(g.GetSelfInfo()->y);//这两行之前若没有位置更新，可删。注：也请检查你们写的代码是否调用了过多的GetSelfInfo函数
	auto b = g.GetCharacters();
	int count = 0;
	for (int i = 0; i < b.size(); i++) {
		if (b[i]->teamID != myid && (!b[i]->isDying)) {
			count++;
			double ax = int(b[i]->x), ay = int(b[i]->y);
			g.Send(3, infobox((int)b[i]->x, (int)b[i]->y, 0, 2));
			g.Send(1, infobox((int)b[i]->x, (int)b[i]->y, 0, 2));
			//作为playerid=2
			while (g.GetSelfInfo()->bulletNum > 0 && (b[i]->propType != (THUAI4::PropType)5 || g.GetSelfInfo()->propType == (THUAI4::PropType)7)) {
				g.Attack(distance(ax, ay, g) / 18, angle(ax, ay, g));
				g.Wait();
			}
			if (ax != my_x || ay != my_y)
			{
				g.MovePlayer(250, atan2(my_y - ay, my_x - ax));
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
				my_x = int(g.GetSelfInfo()->x);
				my_y = int(g.GetSelfInfo()->y);
			}
		}
		else continue;
		if (count > 1)break;
		b = g.GetCharacters();
		count = 0;
		for (int i = 0; i < b.size(); i++)
			if (b[i]->teamID != g.GetSelfInfo()->teamID && (!b[i]->isDying))
				count++;
		g.Wait();
	}//见人不打，赶紧跑
	if (g.MessageAvailable() == true && g.GetSelfInfo()->bulletNum > 0) {
		std::string info0;
		g.TryGetMessage(info0);
		MessageGroup INFO = extractinfo(info0);
		g.Attack(distance(INFO.x, INFO.y, g) / 12, angle(INFO.x, INFO.y, g));
	}
	std::string info;
	int ax = 0, ay = 0; count = 0;
	while (g.GetSelfInfo()->bulletNum > 0 && g.MessageAvailable())
	{
		g.TryGetMessage(info);
		MessageGroup r = extractinfo(info);
		if (r.typeofmessage == 2/*若约定敌人的信息编号为2*/)
		{
			g.Attack((sqrt((int(r.y) - int(my_y)) * (int(r.y) - int(my_y)) + (int(r.x) - int(my_x)) * (int(r.x) - int(my_x)))) / 18, atan2(int(r.y) - int(my_y), int(r.x) - int(my_x)));
		}
		else if (!r.isDying)
		{
			ax += r.x;
			ay += r.y;
			count++;
		}
		ax = ax / count; ay = ay / count;
		g.Wait();
	}
	if (g.GetCounterOfFrames() % 500 == 0)//25s跟进一次战况
		g.MovePlayer(sqrt((ay - int(my_y)) * (ay - int(my_y)) + (ax - int(my_x)) * (ax - int(my_x))) / 8, atan2(my_y - ay, my_x - ax));//向己方中心走去
	g.Wait();

	my_x = (int)g.GetSelfInfo()->x;
	my_y = (int)g.GetSelfInfo()->y;
	if (g.GetSelfInfo()->bulletNum >= 3) restingflag = 0;
	static int cell[9][9] = { -1 };//视野内情况
	if (restingflag == 0) {
		for (int i = -4; i <= 4; i++) {
			for (int j = -4; j <= 4; j++) {
				int k(-1);
				if (my_x / 1000 + i < 1 || my_x / 1000 + i >= 49 || my_y / 1000 + j < 1 || my_y / 1000 + j >= 49) k = -1;//边界以外
				else if (g.GetCellColor(my_x / 1000 + i, my_y / 1000 + j) == THUAI4::ColorType::Invisible) k = -2;//视野以外
				else if (g.GetCellColor(my_x / 1000 + i, my_y / 1000 + j) == mycolor) k = 1;//已涂色区域
				else if (g.GetCellColor(my_x / 1000 + i, my_y / 1000 + j) != mycolor) k = 0;//可涂色区域
				cell[i + 4][j + 4] = k;
			}
		}
		auto wal = g.GetWalls();
		for (int i = 0; i < wal.size(); i++) {
			int xx = (int)wal[i]->x / 1000 - my_x / 1000 + 4, yy = (int)wal[i]->y / 1000 - my_y / 1000 + 4;
			if (xx < 0 || xx>8 || yy < 0 || yy>8) continue;
			cell[xx][yy] = -3;
		}//记录墙的位置
		auto bir = g.GetBirthPoints();
		for (int i = 0; i < bir.size(); i++) {
			int xx = (int)bir[i]->x / 1000 - my_x / 1000 + 4, yy = (int)bir[i]->y / 1000 - my_y / 1000 + 4;
			if (xx < 0 || xx>8 || yy < 0 || yy>8) continue;
			cell[xx][yy] = -4;
		}//记录出生点的位置
		cell[0][0] = cell[0][1] = cell[1][0]
			= cell[0][8] = cell[0][7] = cell[1][8]
			= cell[8][0] = cell[7][0] = cell[8][1]
			= cell[8][8] = cell[8][7] = cell[7][8] = -2;//避免视野边界获取信息出错而撇去四个角的信息
	}
	if (restingflag == 1 || g.GetSelfInfo()->bulletNum == 0) {
		if (g.GetCellColor(my_x / 1000, my_y / 1000) == g.GetSelfTeamColor()) {
			restingflag = 1;
			//g.Wait();
		}
		else {
			restingflag = 0;
			int x = -1, y = -1;
			for (int i = -4; i <= 4; i++) {
				for (int j = -4; j <= 4; j++) {
					if (cell[i + 4][j + 4] == 1) {
						x = my_x + i * 1000;
						y = my_y + j * 1000;
						break;
					}
					else continue;
				}
			}
			if (x > 0 && y > 0) g.MovePlayer(100, angle(x, y, g));//向染色区域前进
			else {
				g.MovePlayer(100, g.GetSelfInfo()->facingDirection + direction(e) / 72);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}//这一串没子弹寻求补给的代码

	{//道具拾取
		auto prp = g.GetProps();
		seekingprp = 0;
		for (int i = 0; i < prp.size(); i++) {
			int dst = distance(prp[i]->x, prp[i]->y, g);
			if (dst == 0) g.Pick(prp[i]->propType);
			if (prp.size() == 1) seekingprp = 1;
			else break;
			my_x = (int)g.GetSelfInfo()->x;
			my_y = (int)g.GetSelfInfo()->y;
			if (restingflag != 1 && dst != 0) {
				g.MovePlayer(dst / 4 > 200 ? 200 : dst / 4, angle(prp[i]->x, prp[i]->y, g));
				std::this_thread::sleep_for(std::chrono::milliseconds(dst / 4 > 200 ? 200 : dst / 4));
				g.Pick(prp[i]->propType);
			}
			if (g.GetSelfInfo()->propType != (THUAI4::PropType)0) {
				g.Use();
				seekingprp = 0;
			}
		}
		if (g.GetSelfInfo()->bulletNum <= 1 || g.GetSelfInfo()->bulletNum >= 11) seekingprp = 0;
	}

	if (distance(my_x, my_y, g) <= 100 && restingflag == 0) blocked += 1;
	else blocked = 0;
	while (blocked) {
		my_x = (int)g.GetSelfInfo()->x;
		my_y = (int)g.GetSelfInfo()->y;
		g.MovePlayer(600, g.GetSelfInfo()->facingDirection + (double)blocked * pi / 4);
		std::this_thread::sleep_for(std::chrono::milliseconds(600));
		if (distance(my_x, my_y, g) <= 100) blocked += 1;
		else blocked = 0;
		g.Wait();
	}//这里是防止卡死的

}
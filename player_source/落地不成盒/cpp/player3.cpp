#include "AI.h"
#include "Constants.h"
#include "Structures.h"


//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = false;

#include <random>
#include <iostream>
#include<cmath>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>
#define STOP std::this_thread::sleep_for(std::chrono::milliseconds(50))
#define place1 19500
#define place3 30500
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */
/******************************     playerID=2    **********************************/
extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业
static double const pi = 3.14159265359;
namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
std::string tran(int x, int y) {//将x,y转化为只有两个字符的字符串。考虑解码。x,y不能调换位置
	char c[3];
	c[0] = x + 49;
	c[1] = y + 49;
	std::string s(&c[0], &c[2]);
	return s;
}
int retranX(std::string s) {//将转化的s重新还原为x
	int x;
	char c[3];
	strcpy(c, s.c_str());
	x = c[0];
	return x - 49;
}
int retranY(std::string s) {//将转化的s重新还原为y
	char c[3];
	int y;
	strcpy(c, s.c_str());
	y = c[1];
	return y - 49;
}
void HappyAttack(GameApi& g, std::shared_ptr<const THUAI4::Character>& self, uint32_t x, uint32_t y)
{
	if (x > self->x)
	{
		if (y > self->y)
			g.Attack(sqrt((self->x - x) * (self->x - x)
				+ (self->y - y) * (self->y - y)) / 12,
				atan(double(y - self->y) / double(x - self->x)));
		else
			g.Attack(sqrt((self->x - x) * (self->x - x)
				+ (self->y - y) * (self->y - y)) / 12,
				2 * pi - atan(double(self->y - y) / double(x - self->x)));
	}
	else if (x < self->x)
	{
		if (y > self->y)
			g.Attack(sqrt((self->x - x) * (self->x - x)
				+ (self->y - y) * (self->y - y)) / 12,
				pi - atan(double(y - self->y) / double(self->x - x)));
		else
			g.Attack(sqrt((self->x - x) * (self->x - x)
				+ (self->y - y) * (self->y - y)) / 12,
				pi + atan(double(self->y - y) / double(self->x - x)));
	}
	else
	{
		if (y > self->y)
			g.Attack(double(y - self->y) / 12, pi / 2);
		else
			g.Attack(double(self->y - y) / 12, -pi / 2);
	}
}
bool IsWall(uint32_t x, uint32_t y, std::vector<std::shared_ptr<const THUAI4::Wall>> walls)
{
	bool iswall = false;
	for (int i = 0; i < walls.size(); i++)
	{
		if (x == walls[i]->x / 1000 && y == walls[i]->y / 1000)
		{
			iswall = true;
			break;
		}
	}
	return iswall;
}
int SearchMinColor(GameApi& g, std::shared_ptr<const THUAI4::Character>& self, uint32_t& x, uint32_t& y)  //找附近自己染色最少的点
{
	static int selfColor[5][5];
	uint32_t selfx = x, selfy = y;
	static auto selfTeamColor = g.GetSelfTeamColor();
	auto walls = g.GetWalls();
	for (int m = 0; m <= 4; m++)
	{
		for (int n = 0; n <= 4; n++)
		{
			selfColor[m][n] = 0;
			for (int i = -1; i <= 1; i++)
				for (int j = -1; j <= 1; j++)
					if (g.GetCellColor(x + m - 2 + i, y + n - 2 + j) == selfTeamColor ||
						IsWall(x + m - 2 + i, y + n - 2 + j, walls) ||
						(x + m - 2 + i > 48 || x + m - 2 + i < 1 || y + n - 2 + j < 1 || y + n - 2 + j > 48))
						selfColor[m][n]++;
		}
	}
	int min = selfColor[0][0];
	for (int i = 0; i < 5; i++)
		for (int j = 0; j < 5; j++)
			if (selfColor[i][j] < min)
			{
				min = selfColor[i][j];
				x = selfx + i - 2;
				y = selfy + j - 2;
			}
	//std::cout << min << std::endl;
	return min;
}

void SearchAndDye(GameApi& g, std::shared_ptr<const THUAI4::Character>& self, bool isrest)
{
	static int frequ = 0;
	uint32_t dyeposx = self->x / 1000, dyeposy = self->y / 1000;
	if (frequ >= 8)
	{
		if (!isrest)
		{
			if (self->bulletNum == 1 && g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor())
			{
				g.Attack(0, 0);
			}
			else if (self->bulletNum > 0)
			{
				if (SearchMinColor(g, self, dyeposx, dyeposy) != 9)
				{
					HappyAttack(g, self, 1000 * dyeposx, 1000 * dyeposy);
					frequ = 0;
				}
			}
		}
	}
	else frequ++;
}
void Rest(GameApi& g, std::shared_ptr<const THUAI4::Character>& self, bool& isrest)
{
	if (!isrest)
	{
		if (self->bulletNum == 0 && g.GetCellColor(self->x / 1000, self->y / 1000) == g.GetSelfTeamColor())
		{
			STOP;
			isrest = true;
			//dyeperiod = 0;  //打光子弹的时候置dyeperiod为0
		}
	}
	else
	{
		STOP;
		isrest = self->bulletNum < self->maxBulletNum&& g.GetCellColor(self->x / 1000, self->y / 1000) == g.GetSelfTeamColor();
	}
}
void WhichDirection0(std::shared_ptr<const THUAI4::Character>& self, int& posflag)
{
	switch (posflag)
	{
	case 0:
	{
		if (self->x > 12000 || self->y < 29000)
			posflag++;
		break;
	}
	case 1:
	{
		if (self->x > 18500 || self->y > 36000)
			posflag++;
		break;
	}
	case 2:
	{
		if (self->x > 43000 || self->y < 31000)
			posflag++;
		break;
	}
	case 3:
	{
		if (self->x < 40000 || self->y > 35000)
			posflag++;
		break;
	}
	case 4:
	{
		if (self->y > 48000)
			posflag++;
		break;
	}
	case 5:
	{
		if (self->x < 33500)
			posflag++;
		break;
	}
	case 6:
	{
		if (self->x < 21500 || self->y < 42500)
			posflag++;
		break;
	}
	case 7:
	{
		if (self->x < 15500 || self->y>48000)
			posflag++;
		break;
	}
	case 8:
	{
		if (self->x < 3600 || self->y < 46500)
			posflag = 0;
		break;
	}
	}
}
void MoveToPos0(GameApi& g, int posflag)
{
	switch (posflag)  //初始：(2500,46500)
	{
	case 0:
	{												//终点
		g.MovePlayer(50, atan(-17500.0 / 9500.0));  //(12000,29000)
		STOP;
		break;
	}
	case 1:
	{
		g.MovePlayer(50, atan(7000.0 / 6500.0));  //(18500,36000)
		STOP;
		break;
	}
	case 2:
	{
		g.MovePlayer(50, atan(-5000.0 / 24500.0)); //(43000,31000)
		STOP;
		break;
	}
	case 3:
	{
		g.MovePlayer(50, atan(-4000.0 / 3000.0) + pi);  //(40000,35000)
		STOP;
		break;
	}
	case 4:
	{
		g.MoveRight(50);  //(40000,48000)
		STOP;
		break;
	}
	case 5:
	{
		g.MoveUp(50);  //(33500,48000)
		STOP;
		break;
	}
	case 6:
	{
		g.MovePlayer(50, atan(5500.0 / 12000.0) + pi);  //(21500,42500)
		STOP;
		break;
	}
	case 7:
	{
		g.MovePlayer(50, atan(-5500.0 / 6000.0) + pi);  //(15500,48000)
		STOP;
		break;
	}
	case 8:
	{
		g.MovePlayer(50, atan(1500.0 / 11900.0) + pi); //(3600,46500)
		STOP;
		break;
	}
	}
}
void WhichDirection1(std::shared_ptr<const THUAI4::Character>& self, int& posflag)
{
	switch (posflag)
	{
	case 0:
	{
		if (self->x < 38500 || self->y < 30000)
			posflag++;
		break;
	}
	case 1:
	{
		if (self->x < 17500 || self->y>36500)
			posflag++;
		break;
	}
	case 2:
	{
		if (self->x < 12500 || self->y < 29500)
			posflag++;
		break;
	}
	case 3:
	{
		if (self->x < 7000 || self->y>43500)
			posflag++;
		break;
	}
	case 4:
	{
		if (self->x > 13500 || self->y > 48000)
			posflag++;
		break;
	}
	case 5:
	{
		if (self->x > 18000)
			posflag++;
		break;
	}
	case 6:
	{
		if (self->x > 26000 || self->y < 42500)
			posflag++;
		break;
	}
	case 7:
	{
		if (self->x > 34000 || self->y > 48000)
			posflag++;
		break;
	}
	case 8:
	{
		if (self->x > 38000)
			posflag++;
		break;
	}
	case 9:
	{
		if (self->x > 46000 || self->y < 46500)
			posflag = 0;
		break;
	}
	}
}
void MoveToPos1(GameApi& g, int posflag)
{
	switch (posflag)			//初始：(46000,46500)
	{
	case 0:
	{
		g.MovePlayer(50, atan(16500.0 / 7500.0) + pi);  //(38500,30000)
		STOP;
		break;
	}
	case 1:
	{
		g.MovePlayer(50, atan(-6500.0 / 21000.0) + pi);  //(17500,36500)
		STOP;
		break;
	}
	case 2:
	{
		g.MovePlayer(50, atan(7000.0 / 5000.0) + pi);  //(12500,29500)
		STOP;
		break;
	}
	case 3:
	{
		g.MovePlayer(50, atan(-14000.0 / 5500.0) + pi);	//(7000,43500)
		STOP;
		break;
	}
	case 4:
	{
		g.MovePlayer(50, atan(4500.0 / 6500.0));	//(13500,48000)
		STOP;
		break;
	}
	case 5:
	{
		g.MoveDown(50);	//(18000,48000)
		STOP;
		break;
	}
	case 6:
	{
		g.MovePlayer(50, atan(-5500.0 / 8000.0));	//(26000,42500)
		STOP;
		break;
	}
	case 7:
	{
		g.MovePlayer(50, atan(5500.0 / 8000.0));	//(34000,48000)
		STOP;
		break;
	}
	case 8:
	{
		g.MoveDown(50);							//(38000,48000)
		STOP;
		break;
	}
	case 9:
	{
		g.MovePlayer(50, atan(-1500.0 / 9000.0));	//(47000,46500)
		STOP;
		break;
	}
	}
}
void DoWithProp(GameApi& g, std::shared_ptr<const THUAI4::Character>& self)
{
	static bool haveprop = false;
	auto prop = g.GetProps();
	if (haveprop)
	{
		haveprop = false;
		g.Use();
	}
	for (int i = 0; i < prop.size(); i++)
	{
		if (prop[i]->x / 1000 == self->x / 1000 && prop[i]->y / 1000 == self->y / 1000)
		{
			if (prop[i]->propType != THUAI4::PropType::Puddle && prop[i]->propType != THUAI4::PropType::MusicPlayer
				&& prop[i]->propType != THUAI4::PropType::Mail)
			{
				g.Pick(prop[i]->propType);
				haveprop = true;
				break;
			}
		}
	}
}
void AI::play(GameApi& g)
{
	//获取信息
	auto self = g.GetSelfInfo();
	auto others = g.GetCharacters();
	static bool isrest = false;
	static int shot = 0;  //射击频率
	static int posflag = 0;
	bool enemy = 0;
	static int bulletposx = 1;  //远程打的子弹偏移位置x
	static int bulletposy = -1;	//远程打的子弹偏移位置y
	static int supplyfre = 10;
	std::string msg;
	if (g.TryGetMessage(msg))
	{
		uint32_t msgx = retranX(msg), msgy = retranY(msg);
		if (msgx != 66)
		{
			if (self->bulletNum > 1)
			{
				if (shot >= 2)
				{
					HappyAttack(g, self, (msgx + 3 * bulletposx) * 1000, (msgy + 3 * bulletposy) * 1000);
					bulletposx--;
					bulletposy++;
					if (bulletposy > 1)
						bulletposy = -1;
					if (bulletposx < -1)
						bulletposx = 1;
					shot = 0;
				}
				else shot++;
			}
		}
		else
		{
			if (self->bulletNum > 1)
			{
				if (supplyfre >= 15)
				{
					supplyfre = 0;
					if (self->teamID == 0)
						HappyAttack(g, self, 2500, place3);
					else HappyAttack(g, self, 47500, place3);
				}
				else supplyfre++;
			}
		}
	}
	if (self->isDying)
		posflag = 0;
	for (int i = 0; i < (int)others.size(); ++i)  //判断附近有无敌人
	{
		if (others[i]->teamID != self->teamID)
		{
			enemy = true;
			break;
		}
		else
			enemy = false;
	}
	DoWithProp(g, self);
	Rest(g, self, isrest);  //先把这一轮isrest的值传回来。
	if (!isrest)  //不在休息就走路
	{
		if (self->teamID == 0)
		{
			WhichDirection0(self, posflag);
			MoveToPos0(g, posflag);
		}
		else
		{
			WhichDirection1(self, posflag);
			MoveToPos1(g, posflag);
		}
	}
	if (!enemy)
		SearchAndDye(g, self, isrest);
	else
	{
		for (int i = 0; i < (int)others.size(); ++i)
		{
			if (others[i]->teamID != self->teamID)
			{
				std::string msg = tran(others[i]->x / 1000, others[i]->y / 1000);
				g.Send(1, msg);
				g.Send(3, msg);
				if (shot >= 2)
					HappyAttack(g, self, others[i]->x, others[i]->y);
				else shot++;
			}
		}
	}
}

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
#include<string>
#include <cstring>
#define STOP std::this_thread::sleep_for(std::chrono::milliseconds(50))
#define place1 19500
#define place3 30500
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */
/************************************     playerID=0    ***************************************/
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
		if (self->y > 12000)
			posflag++;
		break;
	}
	case 1:
	{
		if (self->x > 26000 || self->y > 26000)
			posflag++;
		break;
	}
	case 2:
	{
		if (self->x > 36000)
			posflag++;
		break;
	}
	case 3:
	{
		if (self->x > 42500 || self->y < 9500)
			posflag++;
		break;
	}
	case 4:
	{
		if (self->x < 32500 || self->y < 5000)
			posflag++;
		break;
	}
	case 5:
	{
		if (self->x < 30000 || self->y>17500)
			posflag++;
		break;
	}
	case 6:
	{
		if (self->x < 20000)
			posflag++;
		break;
	}
	case 7:
	{
		if (self->y < 9000)
			posflag++;
		break;
	}
	case 8:
	{
		if (self->x < 3500 || self->y < 4000)
			posflag = 0;
		break;
	}
	}
}
void MoveToPos0(GameApi& g, int posflag)
{
	switch (posflag)
	{
	case 0:
	{
		g.MoveRight(50);  //(4000,12000)
		STOP;
		break;
	}
	case 1:
	{
		g.MovePlayer(50, atan(14500.0 / 22500.0));  //(26500,26500)
		STOP;
		break;
	}
	case 2:
	{
		g.MoveDown(50);  //(36000,26500)
		STOP;
		break;
	}
	case 3:
	{
		g.MovePlayer(50, atan(-17000.0 / 6500.0));  //(42500,9500)
		STOP;
		break;
	}
	case 4:
	{
		g.MovePlayer(50, atan(4500.0 / 10000.0) + pi);   //(32500,5000)
		STOP;
		break;
	}
	case 5:
	{
		g.MovePlayer(50, atan(-12500.0 / 2500.0) + pi);  //(30000,17500)
		STOP;
		break;
	}
	case 6:
	{
		g.MovePlayer(50, pi + atan(2500.0 / 10000.0));  //(20000,15000)
		STOP;
		break;
	}
	case 7:
	{
		g.MoveLeft(50);  //(20000,9000)
		STOP;
		break;
	}
	case 8:
	{
		g.MovePlayer(50, pi + atan(5000.0 / 16500.0));  //(3500,4000)
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
		if (self->x < 35000 || self->y>27500)
			posflag++;
		break;
	}
	case 1:
	{
		if (self->x < 3000 || self->y < 17000)
			posflag++;
		break;
	}
	case 2:
	{
		if (self->x > 6000 || self->y < 7000)
			posflag++;
		break;
	}
	case 3:
	{
		if (self->x > 20000)
			posflag++;
		break;
	}
	case 4:
	{
		if (self->y > 13500)
			posflag++;
		break;
	}
	case 5:
	{
		if (self->x > 27000 || self->y > 17000)
			posflag++;
		break;
	}
	case 6:
	{
		if (self->x > 34500 || self->y < 7500)
			posflag++;
		break;
	}
	case 7:
	{
		if (self->x > 46000 || self->y < 2500)
			posflag = 0;
		break;
	}
	}
}
void MoveToPos1(GameApi& g, int posflag)
{
	switch (posflag)
	{													//初始:(46500,2500),最后走回(46000,2500)
	case 0:
	{
		g.MovePlayer(50, atan(-25000.0 / 11000.0) + pi);  //(35000,27500)
		STOP;
		break;
	}
	case 1:
	{
		g.MovePlayer(50, atan(10500.0 / 32000.0) + pi);  //(3000,17000)
		STOP;
		break;
	}
	case 2:
	{
		g.MovePlayer(50, atan(-10000.0 / 3000.0));  //(6000,7000)
		STOP;
		break;
	}
	case 3:
	{
		g.MoveDown(50);  //(20000,7000)
		STOP;
		break;
	}
	case 4:
	{
		g.MoveRight(50);  //(20000,13500)
		STOP;
		break;
	}
	case 5:
	{
		g.MovePlayer(50, atan(3500.0 / 7000.0));  //(27000,17000)
		STOP;
		break;
	}
	case 6:
	{
		g.MovePlayer(50, atan(-9500.0 / 7500.0));  //(34500,7500)
		STOP;
		break;
	}
	case 7:
	{
		g.MovePlayer(50, atan(-5000.0 / 12000.0));  //(46000,2500)
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
		//std::cout << "I have used" << std::endl;
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
	static int bulletposx = -1;  //远程打的子弹偏移位置x
	static int bulletposy = 1;	//远程打的子弹偏移位置y
	static int supplyfre = 10;  //增援频率，设置为15
	bool enemy = 0;
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
					bulletposx++;
					bulletposy--;
					if (bulletposx > 1)
						bulletposx = -1;
					if (bulletposy < -1)
						bulletposy = 1;
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
						HappyAttack(g, self, 2500, place1);
					else HappyAttack(g, self, 47500, place1);
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

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
#define myplace 30500  //停留位置
#define hisplace 19500
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业
static double const pi = 3.14159265359;
/*******************************     playerID=3    ***************************************/
namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
struct cor
{
	bool isempty;
	int x;
	int y;
};
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
//playerID=3 时的Attack 4V4	(现在拿0试验)
void Dye(GameApi& g, std::shared_ptr<const THUAI4::Character>& self, bool isrest, cor readytodye[16][16])
{
	static int dyex = 0, dyey = 14;
	if (!isrest)
	{
		if (self->bulletNum > 0)
		{
			while (!readytodye[dyex][dyey].isempty)
			{
				dyex++;
				if (dyex >= 16)
				{
					dyex = 0;
					dyey -= 2;
					if (dyey < 0)
						dyey = 14;
				}
			}
			HappyAttack(g, self, readytodye[dyex][dyey].x, readytodye[dyex][dyey].y);
			dyex++;
			if (dyex >= 16)
			{
				dyex = 0;
				dyey -= 2;
				if (dyey < 0)
					dyey = 14;
			}
		}
	}
}
void Rest(GameApi& g, std::shared_ptr<const THUAI4::Character>& self, bool& isrest)
{
	if (!isrest)
	{
		if (self->bulletNum == 0 && g.GetCellColor(self->x / 1000, self->y / 1000) == g.GetSelfTeamColor())
		{
			STOP;
			isrest = true;
		}
	}
	if (isrest)
	{
		if (g.GetCounterOfFrames() < 10200)
		{
			STOP;
			isrest = self->bulletNum < self->maxBulletNum;
		}
		else isrest = self->bulletNum <= 1;
	}
}
void Prepare(GameApi& g, std::shared_ptr<const THUAI4::Character>& self)
{
	static bool start = true;
	if (start)
	{
		if (self->teamID == 0)
			HappyAttack(g, self, 2500, myplace);
		else
			HappyAttack(g, self, 47500, myplace);
		start = false;
	}
	if (self->isDying)
	{
		start = true;
	}
	if (self->y > myplace)
	{
		g.MoveLeft(50);
		STOP;
	}
	if (self->y <= myplace && g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor() && self->bulletNum > 0)
	{
		g.Attack(0, 0);
	}
	else if (self->y <= myplace && g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor() && self->bulletNum == 0)
	{
		std::string supplymsg = tran(55, 55);
		g.Send(1, supplymsg);
	}
}
void AI::play(GameApi& g)
{
	//获取信息
	auto self = g.GetSelfInfo();
	auto others = g.GetCharacters();
	static bool isrest = false;
	static int shot = 0;
	bool enemy = 0;
	srand(rand());
	static int bulletposx = 2;  //远程打的子弹偏移位置x
	static int bulletposy = -2;	//远程打的子弹偏移位置y
	static int supplyfre = 0;   //设置帮助补给子弹周期为15
	std::string msg;
	static cor readytodye[16][16];
	static bool gamestart = true;
	if (gamestart)
	{
		gamestart = false;
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
				if (i == 7 && j == 1) {
					readytodye[i][j].isempty = 0;
				}
				else if (i == 7 && j == 2) {
					readytodye[i][j].isempty = 0;
				}
				else if (i == 8 && j == 1) {
					readytodye[i][j].isempty = 0;
				}
				else if (i == 8 && j == 2) {
					readytodye[i][j].isempty = 0;
				}
				else { readytodye[i][j].isempty = 1; }
				readytodye[i][j].x = 3000 * i + 2500;
				readytodye[i][j].y = 3000 * j + 2500;
			}
		}
	}
	Prepare(g, self);
	if (g.TryGetMessage(msg))
	{	
		uint32_t msgx = retranX(msg), msgy = retranY(msg);
		if (msgx != 55)
		{
			if (self->bulletNum > 1)
			{
				if (shot >= 2)
				{
					HappyAttack(g, self, (msgx + 3 * bulletposx) * 1000, (msgy + 3 * bulletposy) * 1000);
					bulletposy++;
					bulletposx--;
					if (bulletposy > 2)
						bulletposy = -2;
					if (bulletposx < -2)
						bulletposx = 2;
					shot = 0;
				}
				else shot++;
			}
		}
		else if (msgx == 55)
		{
			if (self->bulletNum > 0)
			{
				if (supplyfre >= 15)
				{
					supplyfre = 0;
					if (self->teamID == 0)
						HappyAttack(g, self, 2500, hisplace);
					else HappyAttack(g, self, 47500, hisplace);
				}
				else supplyfre++;
			}
			else if (self->y <= myplace && g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor() && self->bulletNum == 0)
			{
				std::string helpmsg = tran(66, 66);    //(66,66)向移动的传信息，表示两个定点都需要帮助。
				g.Send(2, helpmsg);   //ID=3的角色只向ID=2的求助。
			}
		}
	}
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
	if (g.GetCellColor(self->x / 1000, self->y / 1000) != g.GetSelfTeamColor())
		g.Attack(0, 0);

	if (!enemy)
	{	//往四周染色
		Dye(g, self, isrest, readytodye);
		//回复子弹
		Rest(g, self, isrest);
	}
	else
	{
		for (int i = 0; i < (int)others.size(); ++i)
		{
			std::string helpmsg = tran(others[i]->x / 1000, others[i]->y / 1000);
			g.Send(0, helpmsg);
			g.Send(2, helpmsg);
			if (others[i]->teamID != self->teamID)
				HappyAttack(g, self, others[i]->x, others[i]->y);
		}
	}
}

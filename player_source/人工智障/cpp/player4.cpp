#include "AI.h"
#include "Constants.h"
#include <iostream>
#include <time.h>
#include <math.h>
#define PI 3.1415926

int step = 0;
int hitwall = 0;
//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job3; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * PI);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

void delay_msec(int msec)
{
	clock_t now = clock();
	while (clock() - now < msec);
}

/// @brief  程序延时
/// @param[in] sec : 秒
/// @remark  
/// @return  void
void delay_sec(int sec)//
{
	time_t start_time, cur_time;
	time(&start_time);
	do
	{
		time(&cur_time);
	} while ((cur_time - start_time) < sec);
};

void paint(GameApi& g)
{
	int i;
	auto self = g.GetSelfInfo();
	g.Attack(100, self->facingDirection + PI / 2);
	delay_sec(4);
	g.MovePlayer(3000/6, self->facingDirection + PI / 2);
	delay_sec(1);
	g.Attack(100, self->facingDirection + PI);
	delay_sec(4);
	g.MovePlayer(3000/6, self->facingDirection + PI);
	delay_sec(1);
	if (self->bulletNum<8) delay_sec(4);
	for (i = 0; i < 4; i++)
	{
		g.Attack(100, self->facingDirection + (PI * (2*i + 1)) /4);
		delay_sec(4);
		g.Attack(100, self->facingDirection + (PI * (2 * i + 2)) / 4);
		delay_sec(4);
		g.MovePlayer(1000 / 6, self->facingDirection + (PI*(i+1))/2);
		delay_sec(1);
	}
}

void AI::play(GameApi& g)
{
	int j;
	auto self = g.GetSelfInfo();
	auto pWall = g.GetWalls();
	auto count_Walls = pWall.size();
	double distance_Walls = 5000;
	if (step == 0)
	{
		g.MoveLeft(1000);
		delay_msec(170);
		step++;
	}
	if (count_Walls == 0)
	{
		g.MovePlayer(2000, self->facingDirection);
		if (g.GetSelfTeamColor() != g.GetCellColor(self->x / 1000, self->y / 1000)) paint(g);
		delay_msec(340);
	}
	else if (count_Walls > 0)
	{
		for (int i = 0; i < count_Walls; i++)      //判断最近墙体距离
		{
			if (sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y))) < distance_Walls)
			{
				distance_Walls = sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y)));
				j = i;
			}
		}
		if (distance_Walls < 1500 && step>0)
		{
			if (g.GetSelfTeamColor() != g.GetCellColor(self->x / 1000, self->y / 1000)) paint(g);
			if ((rand() % 100) < 51)
			{
				g.MovePlayer(1000, self->facingDirection + PI / 2);
				delay_msec(170);
			}
			else
			{
				g.MovePlayer(1000, self->facingDirection - PI / 2);
				delay_msec(170);
			}
		}
		if (distance_Walls > 1500)
		{
			g.MovePlayer(1000, self->facingDirection);
			delay_msec(1400);
			hitwall = 0;
		}
	}
	std::cout << g.GetCounterOfFrames() << std::endl;
	std::cout << "I`m at (" << self->x << "," << self->y << ")." << std::endl;
}
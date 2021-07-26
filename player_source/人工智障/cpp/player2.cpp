#include "AI.h"
#include "Constants.h"


//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>
#define PI 3.1425926
int step = 0;
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job3; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{std::random_device{}()};
}
void moveplayer(GameApi& g);
void attack(GameApi& g);
void FindProps(GameApi& g);
void Useprop(GameApi& g);
void delay_msec(int msec);
void delay_sec(int sec);
void paint(GameApi& g);
void AI::play(GameApi &g)
{
	auto self = g.GetSelfInfo();
	moveplayer(g);
	paint(g);
	attack(g);
	FindProps(g);
	//Useprop(g);
	if (self->bulletNum)
	{
		g.Attack(100, self->facingDirection);
	}
	//g.MovePlayer(50, direction(e));
	for (int i = 0; i < 50; i++) 
	{
		for (int j = 0; j < 50; j++) 
		{
			g.GetCellColor(i, j);
		}
	}
	std::cout << "I`m at (" << self->x << "," << self->y << ")." << std::endl;
}
void moveplayer(GameApi& g)
{
	auto self = g.GetSelfInfo();
	auto pWall = g.GetWalls();
	auto count_Walls = pWall.size();
	double distance_Walls = 5000;
	if (count_Walls == 0)
	{
		g.MovePlayer(2000, direction(e));
	}
	if (count_Walls > 0)
	{
		for (int i = 0; i < count_Walls; i++)
		{
			if (sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y))) < distance_Walls)
				distance_Walls = sqrt(((self->x) - (pWall[i]->x)) * ((self->x) - (pWall[i]->x)) + ((self->y) - (pWall[i]->y)) * ((self->y) - (pWall[i]->y)));
		}
		if (distance_Walls < 1001 && step>0)
		{
			if ((rand() % 100) < 51)
				g.MovePlayer(500, self->facingDirection + 3.1415926 / 2);
			else
				g.MovePlayer(500, self->facingDirection - 3.1415926 / 2);
		}
		if(distance_Walls>1001)
			g.MovePlayer(2000, direction(e));
	}
	if (step == 0)
	{
		g.MoveRight(2000);
		step++;
	}
}
void attack(GameApi& g)
{
	auto self = g.GetSelfInfo();
	auto enemy = g.GetCharacters();
	auto num = enemy.size();
	uint32_t distance_Enemies = 0;
	for (int i = 0; i < num; i++)
	{
		if (enemy[i]->teamID != self->teamID)
		{
			distance_Enemies = sqrt(((self->x) - (enemy[i]->x)) ^ 2 + ((self->y) - (enemy[i]->x)) ^ 2);
			if (distance_Enemies < 1001)
			{
				double angle=0;
				uint32_t a;
				a = ((enemy[i]->y) - (self->y)) / ((enemy[i]->x) - (self->x));
				if (((enemy[i]->x) - (self->x)) > 0)
					angle = atan(a);
				if (((enemy[i]->x) - (self->x)) < 0)
					angle = atan(a) + 3.1415926;
				if(((enemy[i]->x) - (self->x)) == 0)
				{
					if (((enemy[i]->y) - (self->y)) > 0) angle = 3.1415926/ 2;
					if (((enemy[i]->y) - (self->y)) < 0) angle = -3.1415926 / 2;
					if (((enemy[i]->y) - (self->y)) == 0) angle = 0;
				}
				g.Attack(0, angle);
			}
		}
	}
}

void FindProps(GameApi& g)
{
	auto self = g.GetSelfInfo();
	auto prop = g.GetProps();
	auto num = prop.size();
	int k=0;
	uint32_t *distance_Props = new uint32_t[num];
	//int *flag = new int[num];
	int Flag=0;
	uint32_t distance_min = 10000;
	/*for (int i = 0; i < num; i++)
	{
		flag[i] = 0;
	}*/
	if (num)
	{
		for (int i = 0; i < num; i++)
		{
			distance_Props[i] = sqrt(((self->x) - (prop[i]->x)) ^ 2 + ((self->y) - (prop[i]->y)) ^ 2);
			/*if (distance_min > distance_Props[i])
			{
				distance_min = distance_Props[i];
				k = i;
			}*/
			//if (prop[i]->propType == THUAI4::PropType::HappyHotPot)
				//flag[i] = 1;
		}
		/*for (int i = 0; i < num; i++)
		{
			if (flag[i] == 1)
			{
				Flag = 1;
				break;
			}
		}*/
		//if (Flag == 1)
		{
			for (int i = 0; i < num; i++)
			{
				if (prop[i]->propType == THUAI4::PropType::HappyHotPot)
				{
					double angle = 0;
					uint32_t a;
					double t;
					a = ((prop[i]->y) - (self->y)) / ((prop[i]->x) - (self->x));
					if (((prop[i]->x) - (self->x)) > 0)
						angle = atan(a);
					if (((prop[i]->x) - (self->x)) < 0)
						angle = atan(a) + 3.1415926;
					if (((prop[i]->x) - (self->x)) == 0)
					{
						if (((prop[i]->y) - (self->y)) > 0) angle = 3.1415926 / 2;
						if (((prop[i]->y) - (self->y)) < 0) angle = -3.1415926 / 2;
						if (((prop[i]->y) - (self->y)) == 0) angle = 0;
					}
					t = (distance_Props[i] / self->moveSpeed) * 1000.0;
					g.MovePlayer(t, angle);
					g.Pick(prop[i]->propType);
					g.Use();
					//Flag++;
					//break;
				}
			}
		}
		/*if (Flag != 0)
		{
			double angle = 0;
			uint32_t a;
			double t;
			a = ((prop[k]->y) - (self->y)) / ((prop[k]->x) - (self->x));
			if (((prop[k]->x) - (self->x)) > 0)
				angle = atan(a);
			if (((prop[k]->x) - (self->x)) < 0)
				angle = atan(a) + 3.1415926;
			if (((prop[k]->x) - (self->x)) == 0)
			{
				if (((prop[k]->y) - (self->y)) > 0) angle = 3.1415926 / 2;
				if (((prop[k]->y) - (self->y)) < 0) angle = -3.1415926 / 2;
				if (((prop[k]->y) - (self->y)) == 0) angle = 0;
			}
			t = (distance_Props[k] / self->moveSpeed) * 1000.0;
			g.MovePlayer(t, angle);
			g.Pick(prop[k]->propType);
		}*/
	}
}
void Useprop(GameApi& g)
{
	auto self = g.GetSelfInfo();
	if (self->propType == THUAI4::PropType::HappyHotPot && self->hp < self->maxHp)
		g.Use();
	else
		g.Use();
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
	//g.Attack(100, self->facingDirection + PI / 2);
	//g.MovePlayer(1000, self->facingDirection + PI / 2);
	//delay_sec(4);
	for (i = 0; i < 4; i++)
	{
		g.Attack(100, self->facingDirection + PI * i / 4+PI);
		delay_sec(4);
	}
}
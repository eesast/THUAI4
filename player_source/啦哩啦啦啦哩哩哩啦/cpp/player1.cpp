

#include "AI.h"
#include "Constants.h"
#include "Structures.h"
//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>
#include<time.h>
/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job2; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}
//以下为我的代码
static int blox = 1;
static int bloy = 1;
static int blockT = 0;
static int flag = 0;
bool block(GameApi& g)
{
	if ((g.GetSelfInfo()->x - blox) / 1000 == 0 && (g.GetSelfInfo()->y - bloy) / 1000 == 0)
		blockT++;
	if (blockT > 10)
	{
		blox = g.GetSelfInfo()->x;
		bloy = g.GetSelfInfo()->y;
		blockT = 0;
		return true;
	}
	else
	{
		blox = g.GetSelfInfo()->x;
		bloy = g.GetSelfInfo()->y;
		return false;
	}
}

bool enemy_around(GameApi& g, std::shared_ptr<const THUAI4::Character>& target)
{
	auto characterinfo = g.GetCharacters();
	auto self = g.GetSelfInfo()->teamID;
	int num = characterinfo.size();
	for (int i = 0; i < num; i++)
	{
		if (num && self != characterinfo[i]->teamID)
		{
			target = characterinfo[i];
			return true;
		}
	}
	return false;
}


bool ahead_wall(GameApi& g, int distance_require, int moveDir)
{
	auto wallinfo = g.GetWalls();
	int b = wallinfo.size();
	auto selfx = g.GetSelfInfo()->x, selfy = g.GetSelfInfo()->y;
	if (moveDir == 0 || moveDir == 2)
	{
		if (b != 0)
			for (int k = 0; k < b; k++)
				if (selfy / 1000 == wallinfo[k]->y / 1000 && (selfx / 1000 - wallinfo[k]->x / 1000) == distance_require * (moveDir - 1))return false;
	}
	else if (moveDir == 1 || moveDir == 3)
	{
		if (b != 0)
			for (int k = 0; k < b; k++)
				if (selfx / 1000 == wallinfo[k]->x / 1000 && (selfy / 1000 - wallinfo[k]->y / 1000) == distance_require * (moveDir - 2))return false;
	}
	return true;
}

double angleCalculate(GameApi& g, int64_t x, int64_t y)
{
	int64_t deltaX = x - g.GetSelfInfo()->x, deltaY = y - g.GetSelfInfo()->y;
	if (deltaX == 0 && deltaY == 0) return 0;
	double angle = atan2(deltaY, deltaX);
	return angle;
}
double distanceCalculate(GameApi& g, int64_t x, int64_t y)
{
	int64_t deltaX = x - g.GetSelfInfo()->x, deltaY = y - g.GetSelfInfo()->y;
	double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
	return distance;
}


void DownProtLeft(GameApi& g);
void LeftProtUp(GameApi& g);
void UpProtRight(GameApi& g);
void RightProtDown(GameApi& g);

void DownProtLeft(GameApi& g)
{
	int light = 0;
	int position1 = g.GetSelfInfo()->y;
	if (!ahead_wall(g, 1, 0))
		light = 1;
	if (light == 0)
	{

		while ((!ahead_wall(g, 1, 3)) && ahead_wall(g, 1, 0))
		{
			g.MoveDown((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());

		}
		int position = g.GetSelfInfo()->x;
		if (!ahead_wall(g, 1, 0))
			light = 1;
		while (g.GetSelfInfo()->x < position + 500 && ahead_wall(g, 1, 0))
		{
			g.MoveDown((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}


		while (g.GetSelfInfo()->y > position1 - 2500 && ahead_wall(g, 1, 3) && light == 0)
		{
			g.MoveLeft((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}


	}

	if (light == 1)
	{
		RightProtDown(g);
		while (g.GetSelfInfo()->y > position1)
		{
			g.MoveLeft((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}
	}
}
void LeftProtUp(GameApi& g)
{
	int light = 0;
	int position1 = g.GetSelfInfo()->x;
	if (!ahead_wall(g, 1, 3))
		light = 1;
	if (light == 0)
	{
		while (!ahead_wall(g, 1, 2) && ahead_wall(g, 1, 3))
		{
			g.MoveLeft((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}
		int position = g.GetSelfInfo()->y;
		if (!ahead_wall(g, 1, 3))
			light = 1;
		while (g.GetSelfInfo()->y > position - 500 && ahead_wall(g, 1, 3))
		{
			g.MoveLeft((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}

		while (g.GetSelfInfo()->x > position1 - 2500 && ahead_wall(g, 1, 2) && light == 0)
		{
			g.MoveUp((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}

	}
	if (light == 1)
	{
		DownProtLeft(g);
		while (g.GetSelfInfo()->x > position1)
		{
			g.MoveUp((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}
	}
}

void UpProtRight(GameApi& g)
{
	int light = 0;
	int position1 = g.GetSelfInfo()->y;
	if (!ahead_wall(g, 1, 2))
		light = 1;
	if (light == 0)
	{

		while (!ahead_wall(g, 1, 1) && ahead_wall(g, 1, 2))
		{
			g.MoveUp((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}
		int position = g.GetSelfInfo()->x;
		if (!ahead_wall(g, 1, 2))
			light = 1;
		while (g.GetSelfInfo()->x > position - 500 && ahead_wall(g, 1, 2))
		{
			g.MoveUp((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}

		while (g.GetSelfInfo()->y < position1 + 2500 && (ahead_wall(g, 1, 1)) && light == 0)
		{
			g.MoveRight((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}


	}
	if (light == 1)
	{
		LeftProtUp(g);
		while (g.GetSelfInfo()->y < position1)
		{
			g.MoveRight((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}
	}
}

void RightProtDown(GameApi& g)
{
	int light = 0;
	int position1 = g.GetSelfInfo()->x;
	if (!ahead_wall(g, 1, 1))
		light = 1;
	if (light == 0)
	{
		while (!ahead_wall(g, 1, 0) && ahead_wall(g, 1, 1))
		{
			g.MoveRight((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}
		int position = g.GetSelfInfo()->y;
		if (!ahead_wall(g, 1, 1))
			light = 1;

		while (g.GetSelfInfo()->y < position + 500 && ahead_wall(g, 1, 1))
		{
			g.MoveRight((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}

		while (g.GetSelfInfo()->x < position1 + 2500 && ahead_wall(g, 1, 0) && light == 0)
		{
			g.MoveDown((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}

	}
	if (light == 1)
	{
		UpProtRight(g);
		while (g.GetSelfInfo()->x < position1)
		{
			g.MoveDown((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
			if (block(g))
				g.MovePlayer(2000, rand());
		}
	}


}

void move(GameApi& g, int a, int b)
{
	if (g.GetSelfInfo()->x / 1000 < a && ahead_wall(g, 1, 0))g.MoveDown((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
	if (g.GetSelfInfo()->y / 1000 < b && ahead_wall(g, 1, 1))g.MoveRight((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
	if (g.GetSelfInfo()->x / 1000 > a && ahead_wall(g, 1, 2))g.MoveUp((1000 * 1000) / g.GetSelfInfo()->moveSpeed);
	if (g.GetSelfInfo()->y / 1000 > b && ahead_wall(g, 1, 3))g.MoveLeft((1000 * 1000) / g.GetSelfInfo()->moveSpeed);

	if (g.GetSelfInfo()->y / 1000 == b && g.GetSelfInfo()->x / 1000 < a && !ahead_wall(g, 1, 0))
		RightProtDown(g);
	if (g.GetSelfInfo()->x / 1000 == a && g.GetSelfInfo()->y / 1000 < b && !ahead_wall(g, 1, 1))
		UpProtRight(g);
	if (g.GetSelfInfo()->y / 1000 == b && g.GetSelfInfo()->x / 1000 > a && !ahead_wall(g, 1, 2))
		LeftProtUp(g);
	if (g.GetSelfInfo()->x / 1000 == a && g.GetSelfInfo()->y / 1000 > b && !ahead_wall(g, 1, 3))
		DownProtLeft(g);

	if (g.GetSelfInfo()->x / 1000 > a && g.GetSelfInfo()->y / 1000 > b && !ahead_wall(g, 1, 2) && !ahead_wall(g, 1, 3))
		DownProtLeft(g);
	if (g.GetSelfInfo()->x / 1000 > a && g.GetSelfInfo()->y / 1000 < b && !ahead_wall(g, 1, 1) && !ahead_wall(g, 1, 2))
		LeftProtUp(g);
	if (g.GetSelfInfo()->x / 1000 < a && g.GetSelfInfo()->y / 1000 > b && !ahead_wall(g, 1, 0) && !ahead_wall(g, 1, 3))
		RightProtDown(g);
	if (g.GetSelfInfo()->x / 1000 < a && g.GetSelfInfo()->y / 1000 < b && !ahead_wall(g, 1, 0) && !ahead_wall(g, 1, 1))
		UpProtRight(g);
}






void xymove(GameApi& g, int a, int b)
{
	g.MovePlayer((distanceCalculate(g, a, b) * 1000) / g.GetSelfInfo()->moveSpeed, angleCalculate(g, a, b));
}

void fight(GameApi& g, std::shared_ptr<const THUAI4::Character>& target)
{
	double beta = angleCalculate(g, target->x, target->y);
	if (distanceCalculate(g, target->x, target->y) <= 4000)g.Attack(distanceCalculate(g, target->x, target->y), beta);
}


bool pick(GameApi& g)
{
	int blocktime = 0;
	auto prop = g.GetProps();
	if (prop.size())
	{
		while (((prop[0]->x / 1000 != g.GetSelfInfo()->x / 1000) || (prop[0]->y / 1000 != g.GetSelfInfo()->y / 1000)) && blocktime < 10)
		{
			move(g, prop[0]->x / 1000, prop[0]->y / 1000);
			blocktime++;
		}
		if (blocktime == 10)
		{
			g.MovePlayer(2000, rand());
			return false;
		}
		if ((prop[0]->x / 1000 == g.GetSelfInfo()->x / 1000) && (prop[0]->y / 1000 == g.GetSelfInfo()->y / 1000))
		{
			g.Pick(prop[0]->propType);
			g.Use();
			return true;

		}
	}

}

void AI::play(GameApi& g)
{
	srand(time(NULL));
	std::shared_ptr<const THUAI4::Character> target;

	if (g.GetProps().size())pick(g);
	if (g.GetSelfInfo()->bulletNum > 2)
	{

		if (flag == 0)
		{
			move(g, 25, 25);
			if (block(g))
				g.MovePlayer(2000, rand());
			if (distanceCalculate(g, 25000, 25000) < 2000)
				flag = 1;
		}
		if (flag == 1)
		{
			move(g, 7, 10);
			if (block(g))
				g.MovePlayer(2000, rand());
			if (distanceCalculate(g, 7000, 10000) < 2000)
				flag = 2;
		}
		if (flag == 2)
		{
			move(g, 7, 45);
			if (block(g))
				g.MovePlayer(2000, rand());
			if (distanceCalculate(g, 7000, 45000) < 2000)
				flag = 3;
		}
		if (flag == 3)
		{
			move(g, 42, 42);
			if (block(g))
				g.MovePlayer(2000, rand());
			if (distanceCalculate(g, 42000, 42000) < 2000)
				flag = 4;
		}
		if (flag == 4)
		{
			move(g, 45, 5);
			if (block(g))
				g.MovePlayer(2000, rand());
			if (distanceCalculate(g, 45000, 5000) < 2000)
				flag = 0;
		}

		if (enemy_around(g, target))
		{
			fight(g, target);

		}
		else if (!enemy_around(g, target))
		{
			if (g.GetCellColor(g.GetSelfInfo()->x / 1000, g.GetSelfInfo()->y / 1000) != g.GetSelfTeamColor())
				g.Attack(1, rand());
		}
	}
	if (g.GetSelfInfo()->bulletNum < 3)
	{
		if (g.GetCellColor(g.GetSelfInfo()->x / 1000, g.GetSelfInfo()->y / 1000) != g.GetSelfTeamColor())
			g.Attack(1, rand());
	}
	if (!ahead_wall(g, 1, 1) && !ahead_wall(g, 1, 3))
		g.MoveDown(5000);
	if (distanceCalculate(g, 25000, 1000) < 500)
		g.MoveDown(10000);
	//if (distanceCalculate(g, 19000, 10000) < 1000)
		//g.MoveLeft(1500);
}
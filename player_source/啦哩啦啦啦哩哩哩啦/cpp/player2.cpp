#include "AI.h"
#include "Constants.h"

extern const bool asynchronous = true;

#include <random>
#include <iostream>

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; 

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

uint32_t timeCalculate(GameApi& g, int64_t x, int64_t y)
{
	int64_t deltaX = x - g.GetSelfInfo()->x, deltaY = y - g.GetSelfInfo()->y;
	uint32_t time_in_mili;
	double distance = sqrt(deltaX * deltaX + deltaY * deltaY);
	time_in_mili = (uint32_t)distance / 12;
	return time_in_mili;
}

double angleCalculate(GameApi& g, int64_t x, int64_t y)
{
	int64_t deltaX = x - g.GetSelfInfo()->x, deltaY = y - g.GetSelfInfo()->y;
	if (deltaX == 0 && deltaY == 0) return 0;
	double angle = atan2(deltaY, deltaX);
	return angle;
}

bool ahead_wall(GameApi& g, int distance_require, int moveDir)
{
	auto wallinfo = g.GetWalls();
	int b = wallinfo.size();
	auto selfx = g.GetSelfInfo()->x, selfy = g.GetSelfInfo()->y;
	if (moveDir == 0 || moveDir == 2)
	{
		if (b)
			for (int k = 0; k < b; k += 1)
				if (selfy / 1000 == wallinfo[k]->y / 1000 && (selfx / 1000 - wallinfo[k]->x / 1000) == distance_require * (moveDir - 1)) return false;
	}
	else if (moveDir == 1 || moveDir == 3)
	{
		if (b)
			for (int k = 0; k < b; k += 1)
				if (selfx / 1000 == wallinfo[k]->x / 1000 && (selfy / 1000 - wallinfo[k]->y / 1000) == distance_require * (moveDir - 2)) return false;
	}
	return true;
}

bool ahead_bullet(GameApi& g, int distance_require, int moveDir)
{
	auto bulletinfo = g.GetBullets();
	auto selfbullet = g.GetSelfInfo()->teamID;
	int a = bulletinfo.size();
	auto selfx = g.GetSelfInfo()->x, selfy = g.GetSelfInfo()->y;
	if (moveDir == 0 || moveDir == 2)
	{
		if (a)
			for (int k = 0; k < a; k += 1)
				if (selfy / 1000 == bulletinfo[k]->y / 1000 && (selfx / 1000 - bulletinfo[k]->x / 1000) == distance_require * (moveDir - 1) && selfbullet != bulletinfo[k]->teamID) return false;
	}
	else if (moveDir == 1 || moveDir == 3)
	{
		if (a)
			for (int k = 0; k < a; k += 1)
				if (selfx / 1000 == bulletinfo[k]->x / 1000 && (selfy / 1000 - bulletinfo[k]->y / 1000) == distance_require * (moveDir - 2) && selfbullet != bulletinfo[k]->teamID) return false;
	}
	return true;
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

int Tictactoe(GameApi &g,int X,int Y)
{
	int counter=0;
	for(int i=0;i<2;i++)
		for (int j = 0; j < 2; j++)
			counter += (g.GetCellColor(X,Y)!=g.GetSelfTeamColor()) && (X>=0) && (X<=50)&&(Y>=0)&&(Y<=0);
	return counter;
}
int max_tic(GameApi& g)
{
	auto self= g.GetSelfInfo();
	int selfX = self->x / 1000, selfY = self->y / 1000;
	int finalX = selfX, finalY = selfY, finalNum = Tictactoe(g, selfX, selfY), i, j;
	for(i=-4;i<5;i++)
		for (j = -4; j < 5; j++)
			if (finalNum < Tictactoe(g,selfX+i,selfY+j))
			{
				finalX = selfX + i;
				finalY = selfY + j;
			}
	return 100 * finalX + finalY;
}
void MakeMessage(int raw, std::string message)
{
	message[0] = raw / 100;
	message[1] = raw % 100;
}
std::string message_1;
//以上三个函数主要实现转换出一个视野内非本队颜色Cell最多的九宫格中心的信息，复用性较差，可待改进

int range = 4;
static int64_t  atArea = 0;
static const double alpha[4] = { 0,0.5 * 3.1415926,3.1415926,1.5 * 3.1415926 };

void AI::play(GameApi& g)
{
	static const int64_t O_prime[8][2] = { {1000,1000},{25000,25000},{37000,1000},{13000,25000},{1000,37000}
	,{25000,13000},{37000,37000},{13000,13000} };
	static int corner = 2 * (g.GetSelfInfo()->x > 20000) + 4 * (g.GetSelfInfo()->y > 20000);
	std::shared_ptr<const THUAI4::Character> target=NULL;
	if (enemy_around(g, target))
	{
		if (g.GetSelfInfo()->bulletNum)
			g.Attack(timeCalculate(g, target->x, target->y), angleCalculate(g, target->x, target->y));
	}
	else if (g.GetSelfInfo()->bulletNum >= 12)
	{
		int twelve =11;
		{
			while (twelve--)
			{
				int64_t x = int64_t(1500 + O_prime[corner % 8][0]) + int64_t((atArea % range) * 3000);
				int64_t y = int64_t(1500 + O_prime[corner % 8][1]) + int64_t(((atArea / range) % range) * 3000);
				g.Attack(timeCalculate(g, x, y), angleCalculate(g, x, y));
				corner=atArea/16;
				atArea += 1;
			}
			
		}
	}
	else g.Wait();
}
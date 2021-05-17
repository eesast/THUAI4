#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>
#include <thread>
#include <cmath>


extern const THUAI4::JobType playerJob = THUAI4::JobType::Job1; //选手职业，选手 !!必须!! 定义此变量来选择职业

//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
//HappyMan 2 (from right to left)
const double PI = 3.14159265358979323846264338;
const double v = 12.0;
int flag_time = 0;
int time0 = 0;
int time1 = 0;
int time_now = 0;
int flag_attack = 1;
class xy
{
public:
	double x;
	double y;
};
xy aim[16][16];
bool start = 0;
int k = 0, m = 0;
double selfx, selfy;

void Attack(GameApi& g, double x, double y)
{
	uint32_t t = (uint32_t)sqrt((selfx - x) * (selfx - x) + (selfy - y) * (selfy - y)) / v;
	double angle = atan2(y - selfy, x - selfx);
	g.Attack(t, angle);
}

void AI::play(GameApi& g)
{
	if (start == 0)
	{
		start = 1;
		for (int i = 0; i < 16; i++)
		{
			for (int j = 0; j < 16; j++)
			{
				aim[i][15 - j].x = 2500.0 + 3000.0 * double(i);
				aim[i][15 - j].y = 2500.0 + 3000.0 * double(j);
			}
		}
		selfx = g.GetSelfInfo()->x;
		selfy = g.GetSelfInfo()->y;
	}
	int flag_attackman = 1;
	while (flag_attackman)
	{
		flag_attackman = 0;
		auto character = g.GetCharacters();
		if (character.size() > 1)
		{
			for (int i = 1; i < character.size(); i++)
			{
				if (character[i]->teamID == g.GetSelfInfo()->teamID)
				{
					flag_attackman = 0;
				}
				else
				{
					flag_attackman = 1;
					Attack(g, character[i]->x, character[i]->y);
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
			}
		}
	}
	if (g.GetSelfInfo()->bulletNum)
	{
		Attack(g, aim[k][m].x, aim[k][m].y);
		k++;
		if (k == 16)
		{
			k = 0;
			m++;
			if (m == 16)
			{
				m = 0;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
	{//获取当前时间
		while (!flag_time)
		{
			time0 = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			flag_time = 1;
		}
		flag_time++;
		time1 = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		time_now = time1 - time0;
		std::cout << time_now << std::endl << time1 << std::endl << flag_time << std::endl;
	}

	{//虐泉

		while (time_now % 50 == 1 && flag_attack)
		{
			int x_enemy = 0;
			int y_enemy = 0;
			if (selfx < 25000 && selfy < 25000)
			{
				x_enemy = 47500;
				y_enemy = 2500;
			}
			else if (selfx < 25000 && selfy > 25000)
			{
				x_enemy = 47500;
				y_enemy = 47500;
			}
			else if (selfx > 25000 && selfy < 25000)
			{
				x_enemy = 2500;
				y_enemy = 2500;
			}
			else if (selfx > 25000 && selfy > 25000)
			{
				x_enemy = 2500;
				y_enemy = 47500;
			}
			if (g.GetSelfInfo()->bulletNum)
			{
				Attack(g, double(x_enemy), double(y_enemy));
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				flag_attack = 0;
			}

			std::cout << x_enemy << std::endl << y_enemy << std::endl;
		}

		if (time_now % 50 == 2)
		{
			flag_attack = 1;
		}
	}
}

#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = true;

#include <random>
#include <iostream>
#include <thread>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job3; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{ std::random_device{}() };
}

//↓↓↓
const double PI = 3.14159265358979323846264338;
int find_count = 0;
int flag = 0;

void AI::play(GameApi& g)
{
	int switch_value = 1;
	if (g.GetSelfInfo()->teamID == 0)
	{
		switch_value = 1;
	}//todo,1-4
	if (g.GetSelfInfo()->teamID == 1)
	{
		switch_value = 2;
	}//todo,1-4
	int x_now = g.GetSelfInfo()->x;
	int y_now = g.GetSelfInfo()->y;
	while (!find_count)//find corner
	{
		while (x_now > 1500 && x_now < 48500 || y_now > 1500 && y_now < 48500)
		{
			switch (switch_value)
			{
			case 3://left up
			{
				g.MoveUp(100);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				g.MoveLeft(100);
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				x_now = g.GetSelfInfo()->x;
				y_now = g.GetSelfInfo()->y;
				break;
			}
			case 1://right up
			{
				g.MoveUp(150);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				g.MoveRight(150);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				x_now = g.GetSelfInfo()->x;
				y_now = g.GetSelfInfo()->y;
				break;
			}
			case 4://left down
			{
				g.MoveDown(150);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				g.MoveLeft(150);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				x_now = g.GetSelfInfo()->x;
				y_now = g.GetSelfInfo()->y;
				break;
			}
			case 2://right down
			{
				g.MoveDown(150);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				g.MoveRight(150);
				std::this_thread::sleep_for(std::chrono::milliseconds(150));
				x_now = g.GetSelfInfo()->x;
				y_now = g.GetSelfInfo()->y;
				break;
			}
			}
			find_count = 1;
		}

		g.Attack(0, -PI / 2);
		break;


	}
	while (find_count)//来回移动
	{
		switch (switch_value)
		{
		case 1:
		{

			int count = 0;
			while (flag % 2 == 1 && flag % 3 != 0)
			{
				g.MoveRight(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0,atan2(character[i]->y-g.GetSelfInfo()->y,character[i]->x-g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				count++;
				if (count >= 25)
				{
					count = 0;
					if (g.GetSelfInfo()->bulletNum > 1)
					{
						switch (rand() % 2)
						{
						case 0:
							g.Attack(0, PI / 6); break;
						case 1:
							g.Attack(0, -PI / 6); break;
						}
					}
				}
				if (g.GetSelfInfo()->y == 48500)
				{
					flag++;
					g.Attack(0, PI * 1.5);
				}
				if (g.GetSelfInfo()->isDying)
				{
					flag++;
					find_count = 0;
				}
			}
			while (flag % 2 == 1 && flag % 3 == 0)
			{
				g.MoveRight(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0, atan2(character[i]->y - g.GetSelfInfo()->y, character[i]->x - g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				if (g.GetSelfInfo()->y <= 4680 && g.GetSelfInfo()->y >= 4320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(10, 32));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 21680 && g.GetSelfInfo()->y >= 21320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-11, 28));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 9680 && g.GetSelfInfo()->y >= 9320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(36, 47));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 16680 && g.GetSelfInfo()->y >= 16320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(22, 47));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 36680 && g.GetSelfInfo()->y >= 36320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-35, 40));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 45680 && g.GetSelfInfo()->y >= 45320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-44, 38));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 13680 && g.GetSelfInfo()->y >= 13320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(20, 47));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y == 48500 && g.GetSelfInfo()->bulletNum > 1)
				{
					flag++;
					g.Attack(0, PI * 1.5);
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->isDying)
				{
					flag++;
					find_count = 0;
				}
			}
			while (flag % 2 == 0 && flag % 3 != 0)
			{
				g.MoveLeft(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0, atan2(character[i]->y - g.GetSelfInfo()->y, character[i]->x - g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				count++;
				if (count >= 25)
				{
					count = 0;
					if (g.GetSelfInfo()->bulletNum > 1)
					{
						switch (rand() % 2)
						{
						case 0:
							g.Attack(0, PI / 6); break;
						case 1:
							g.Attack(0, -PI / 6); break;
						}
					}
				}
				if (g.GetSelfInfo()->y == 1500)
				{
					flag += 1;
					g.Attack(0, PI / 2);

				}
				if (g.GetSelfInfo()->isDying)
				{
					flag += 1;
					find_count = 0;
				}
			}
			while (flag % 2 == 0 && flag % 3 == 0)
			{
				g.MoveLeft(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0, atan2(character[i]->y - g.GetSelfInfo()->y, character[i]->x - g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				if (g.GetSelfInfo()->y <= 4680 && g.GetSelfInfo()->y >= 4320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(10, 32));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 13680 && g.GetSelfInfo()->y >= 13320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(20, 47)); std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 21680 && g.GetSelfInfo()->y >= 21320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-11, 28));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 9680 && g.GetSelfInfo()->y >= 9320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(36, 47));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 16680 && g.GetSelfInfo()->y >= 16320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(22, 47));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 36680 && g.GetSelfInfo()->y >= 36320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-35, 40));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 45680 && g.GetSelfInfo()->y >= 45320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-44, 38));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y == 1500)
				{
					flag++;
					g.Attack(0, PI * 1.5);
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->isDying)
				{
					flag++;
					find_count = 0;
				}
			}
			break;
		}
		case 2:
		{
			int count = 0;
			while (flag % 2 == 1 && flag % 3 != 0)
			{
				g.MoveRight(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0, atan2(character[i]->y - g.GetSelfInfo()->y, character[i]->x - g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				count++;
				if (count >= 25)
				{
					count = 0;
					if (g.GetSelfInfo()->bulletNum > 1)
					{
						switch (rand() % 2)
						{
						case 0:
							g.Attack(0, 5 * PI / 6); break;
						case 1:
							g.Attack(0, -5 * PI / 6); break;
						}
					}
				}
				if (g.GetSelfInfo()->y == 48500)
				{
					flag++;
					g.Attack(0, PI * 1.5);
				}
				if (g.GetSelfInfo()->isDying)
				{
					flag++;
					find_count = 0;
				}
			}
			while (flag % 2 == 1 && flag % 3 == 0)
			{
				g.MoveRight(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0, atan2(character[i]->y - g.GetSelfInfo()->y, character[i]->x - g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				if (g.GetSelfInfo()->y <= 44680 && g.GetSelfInfo()->y >= 44320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-4, -6));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 2680 && g.GetSelfInfo()->y >= 2320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(12, -25));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 7680 && g.GetSelfInfo()->y >= 7320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(5, -5));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 11680 && g.GetSelfInfo()->y >= 11320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(3, -4));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 20680 && g.GetSelfInfo()->y >= 20320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(1, -5));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 42680 && g.GetSelfInfo()->y >= 42320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-7, -6));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}

				if (g.GetSelfInfo()->y <= 39680 && g.GetSelfInfo()->y >= 39320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-5, -6));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 34680 && g.GetSelfInfo()->y >= 34320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-2, -5));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y == 48500)
				{
					flag++;
					g.Attack(0, PI * 1.5);
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->isDying)
				{
					flag++;
					find_count = 0;
				}
			}
			while (flag % 2 == 0 && flag % 3 != 0)
			{
				g.MoveLeft(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0, atan2(character[i]->y - g.GetSelfInfo()->y, character[i]->x - g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				count++;
				if (count >= 25)
				{
					count = 0;
					if (g.GetSelfInfo()->bulletNum > 1)
					{
						switch (rand() % 2)
						{
						case 0:
							g.Attack(0, 5 * PI / 6); break;
						case 1:
							g.Attack(0, -5 * PI / 6); break;
						}
					}
				}
				if (g.GetSelfInfo()->y == 1500)
				{
					flag += 1;
					g.Attack(0, PI / 2);

				}
				if (g.GetSelfInfo()->isDying)
				{
					flag += 1;
					find_count = 0;
				}
			}
			while (flag % 2 == 0 && flag % 3 == 0)
			{
				g.MoveLeft(60);
				std::this_thread::sleep_for(std::chrono::milliseconds(60));
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
								g.Attack(0, atan2(character[i]->y - g.GetSelfInfo()->y, character[i]->x - g.GetSelfInfo()->x));
								std::this_thread::sleep_for(std::chrono::milliseconds(50));
							}
						}
					}
				}
				if (g.GetSelfInfo()->y <= 44680 && g.GetSelfInfo()->y >= 44320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-4, -6));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 2680 && g.GetSelfInfo()->y >= 2320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(12, -25));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 7680 && g.GetSelfInfo()->y >= 7320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(5, -5));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 11680 && g.GetSelfInfo()->y >= 11320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(3, -4));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 20680 && g.GetSelfInfo()->y >= 20320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(1, -5));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 42680 && g.GetSelfInfo()->y >= 42320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-7, -6));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 39680 && g.GetSelfInfo()->y >= 39320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-5, -6));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y <= 34680 && g.GetSelfInfo()->y >= 34320 && g.GetSelfInfo()->bulletNum > 1)
				{
					g.Attack(0, atan2(-2, -5));
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->y == 1500)
				{
					flag++;
					g.Attack(0, PI * 1.5);
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
				}
				if (g.GetSelfInfo()->isDying)
				{
					flag++;
					find_count = 0;
				}
			}
			break;
		}


		}
	}
}

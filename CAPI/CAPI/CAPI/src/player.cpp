#include "AI.h"
#include "Constants.h"

//为假则play()调用期间游戏状态更新阻塞，为真则只保证当前游戏状态不会被状态更新函数与GameApi的方法同时访问
extern const bool asynchronous = false;

#include <random>
#include <iostream>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

extern const THUAI4::JobType playerJob = THUAI4::JobType::Job0; //选手职业，选手 !!必须!! 定义此变量来选择职业

namespace
{
	[[maybe_unused]] std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
	[[maybe_unused]] std::default_random_engine e{std::random_device{}()};
}

void AI::play(GameApi &g)
{
	auto self = g.GetSelfInfo();
	if (self->bulletNum)
	{
		g.Attack(100, direction(e));
	}
	g.MovePlayer(50, direction(e));
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++) {
			g.GetCellColor(i, j);
		}
	}
	std::cout << g.GetCounterOfFrames() << std::endl;
	std::cout << "I`m at (" << self->x << "," << self->y << ")." << std::endl;
}

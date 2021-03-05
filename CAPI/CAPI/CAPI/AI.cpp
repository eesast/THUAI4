#include "AI.h"
#include "Constants.h"

#include <iostream>
#include <chrono>
#include <thread>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

void AI::play(GameApi& g)
{
	std::cout << "I`m playing......" << std::endl;
	if (g.MessageAvailable()) {
		std::string str;
		g.TryGetMessage(str);
		std::cout << "Oh! I get a message:" << str << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

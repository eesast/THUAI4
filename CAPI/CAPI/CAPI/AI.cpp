#include "AI.h"
#include "Constants.h"

/* ���� VS2019 ��Ŀ�����п��� C++17 ��׼��/std:c++17 */

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

#include "AI.h"
#include "Constants.h"

/* ���� VS2019 ��Ŀ�����п��� C++17 ��׼��/std:c++17 */

void AI::play()
{
	std::cout << "I`m playing......" << std::endl;
	std::cout << "Frame:" << GetSelfInfo().guid << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

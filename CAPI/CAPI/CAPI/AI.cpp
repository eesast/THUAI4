#include"AI.h"
void AI::play()
{
	std::cout << "I`m playing......" << std::endl;
	std::cout << "Frame:" << getSelfInfo().guid << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}
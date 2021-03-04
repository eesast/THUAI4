#include"AI.h"
void AI::play()
{
	std::cout << "I`m playing......" << std::endl;
	if (MessageAvailable()) {
		std::string str;
		TryGetMessage(str);
		std::cout << "Oh! I get a message:" << str << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
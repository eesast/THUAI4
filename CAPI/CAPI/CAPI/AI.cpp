#include "AI.h"
#include "Constants.h"

#include <random>

/* 请于 VS2019 项目属性中开启 C++17 标准：/std:c++17 */

std::uniform_real_distribution<double> direction(0, 2 * 3.1415926);
std::default_random_engine e;

void AI::play(GameApi& g)
{

	g.MovePlayer(50, direction(e));
	if (g.GetSelfInfo().bulletNum) {
		g.Attack(100, direction(e));
	}
}

std::shared_ptr<AIBase> CreateAI() {
	return std::make_shared<AI>();
}

int main(int argc, char** argv) {
	return thuai4_main(argc, argv, CreateAI);
}

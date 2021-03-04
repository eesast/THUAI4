#pragma once
#include <cstdint>
namespace Constants
{
	static const int maxlength = 1000;//Message2Bytes时缓冲区的长度，够大就行
	static const int32_t MessageToClient = 0;
	static const int32_t MessageToServer = 1;
	static const int32_t MessageToOneClient = 2;
	//视野半径 **数值是我随手写的**
	static const uint64_t SightRadius = 5000;
	static const uint64_t SightRadiusSquared = SightRadius * SightRadius;
	static const uint32_t numOfGridPerCell = 1000;
}


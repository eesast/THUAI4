#pragma once
#include <cstdint>
namespace Constants
{
	static const int maxlength = 1000;//Message2Bytesʱ�������ĳ��ȣ��������
	static const int32_t MessageToClient = 0;
	static const int32_t MessageToServer = 1;
	static const int32_t MessageToOneClient = 2;
	//��Ұ�뾶 **��ֵ��������д��**
	static const uint64_t SightRadius = 5000;
	static const uint64_t SightRadiusSquared = SightRadius * SightRadius;
	static const uint32_t numOfGridPerCell = 1000;
}


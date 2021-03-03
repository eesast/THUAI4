#pragma once

#ifndef CONSTANTS_H

#define CONSTANTS_H

#include <cstdint>
#include <numeric>

#define G_SCI static constexpr inline
#define GF_CI constexpr inline
#define _NODISCARD [[nodiscard]]

namespace Constants
{	
	G_SCI int maxlength = 64;		//发送信息最大长度
	G_SCI int32_t MessageToClient = 0;
	G_SCI int32_t MessageToServer = 1;
	G_SCI int32_t MessageToOneClient = 2;
}

#undef GF_CI
#undef G_SCI

#endif //!CONSTANTS_H

#pragma once

#ifndef CONSTANTS_H

#define CONSTANTS_H

#include <cstdint>
#include <numeric>

#define G_SCI static const constexpr inline
#define GF_CI constexpr inline
#define _NODISCARD [[nodiscard]]

namespace Constants
{

	G_SCI uint64_t SightRadius = 5000;

	G_SCI uint64_t SightRadiusSquared = SightRadius * SightRadius;

	G_SCI uint32_t numOfGridPerCell = 1000;

	G_SCI uint32_t numOfPlayer = 4;

	G_SCI uint32_t numOfTeam = 2;

}

#undef GF_CI
#undef G_SCI

#endif //!CONSTANTS_H

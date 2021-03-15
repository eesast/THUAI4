#pragma once

#ifndef CONSTANTS_H

#define CONSTANTS_H

#include <cstdint>
#include <numeric>
#include <utility>

#define M_SCI static const constexpr inline
#define MF_SCI static constexpr inline

namespace Constants
{
	
	struct Map
	{
		using XYPosition = ::std::pair<std::int32_t, std::int32_t>;

		M_SCI std::uint64_t sightRadius = 5000;
		M_SCI std::uint64_t sightRadiusSquared = sightRadius * sightRadius;
		M_SCI std::int32_t numOfGridPerCell = 1000;

		[[nodiscard]] MF_SCI auto CellToGrid(int x, int y) noexcept
		{
			return std::make_pair<std::int32_t, std::int32_t>(x * numOfGridPerCell + numOfGridPerCell / 2, y * numOfGridPerCell + numOfGridPerCell / 2);
		}

		[[nodiscard]] MF_SCI std::int32_t GridToCellX(XYPosition pos) noexcept
		{
			return pos.first / numOfGridPerCell;
		}

		[[nodiscard]] MF_SCI std::int32_t GridToCellY(XYPosition pos) noexcept
		{
			return pos.second / numOfGridPerCell;
		}
	};

	struct BulletMoveSpeed
	{
	private:

		M_SCI std::int32_t basicBulletMoveSpeed = Map::numOfGridPerCell * 6;

	public:

		M_SCI std::int32_t bullet0 = basicBulletMoveSpeed;
		M_SCI std::int32_t bullet1 = basicBulletMoveSpeed * 2;
		M_SCI std::int32_t bullet2 = basicBulletMoveSpeed / 2;
		M_SCI std::int32_t bullet3 = basicBulletMoveSpeed / 2;
		M_SCI std::int32_t bullet4 = basicBulletMoveSpeed * 4;
		M_SCI std::int32_t bullet5 = basicBulletMoveSpeed;
		M_SCI std::int32_t bullet6 = basicBulletMoveSpeed;
	};

	struct PropTimeInSeconds
	{

	private:

		M_SCI std::int32_t propTimeInSeconds = 30;
		M_SCI std::int32_t mineTimeInSeconds = 60;

	public:

		M_SCI std::int32_t bike = propTimeInSeconds;
		M_SCI std::int32_t amplifier = propTimeInSeconds;
		M_SCI std::int32_t jinKeLa = propTimeInSeconds;
		M_SCI std::int32_t rice = propTimeInSeconds;
		M_SCI std::int32_t shield = propTimeInSeconds;
		M_SCI std::int32_t totem = propTimeInSeconds;
		M_SCI std::int32_t spear = propTimeInSeconds;

		M_SCI std::int32_t dirt = mineTimeInSeconds;
		M_SCI std::int32_t attenuator = mineTimeInSeconds;
		M_SCI std::int32_t divider = mineTimeInSeconds;
	};

	struct ID
	{
		M_SCI std::int64_t invalidGUID = std::numeric_limits<std::int64_t>::max();
		M_SCI std::int64_t noneGUID = std::numeric_limits<std::int64_t>::min();
	};

}

#undef MF_SCI
#undef M_SCI

#endif //!CONSTANTS_H

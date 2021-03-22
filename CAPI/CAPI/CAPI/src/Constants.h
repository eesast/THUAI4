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
		using XYPosition = ::std::pair<std::int32_t, std::int32_t>;				// 坐标结构体

		M_SCI std::uint64_t sightRadius = 5000;									// 视野半径
		M_SCI std::uint64_t sightRadiusSquared = sightRadius * sightRadius;		// 视野半径的平方
		M_SCI std::int32_t numOfGridPerCell = 1000;								// 每个格子边长

		[[nodiscard]] MF_SCI auto CellToGrid(int x, int y) noexcept				// 获取指定格子中心的坐标
		{
			return std::make_pair<std::int32_t, std::int32_t>(x * numOfGridPerCell + numOfGridPerCell / 2, y * numOfGridPerCell + numOfGridPerCell / 2);
		}

		[[nodiscard]] MF_SCI std::int32_t GridToCellX(XYPosition pos) noexcept	// 获取指定坐标点所位于的格子的 X 序号
		{
			return pos.first / numOfGridPerCell;
		}

		[[nodiscard]] MF_SCI std::int32_t GridToCellY(XYPosition pos) noexcept	// 获取指定坐标点所位于的格子的 Y 序号
		{
			return pos.second / numOfGridPerCell;
		}
	};

	// 各种子弹的移动速度（每秒移动的坐标数）

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

		M_SCI std::int32_t ordinaryBullet = bullet0;
		M_SCI std::int32_t happyBullet = bullet1;
		M_SCI std::int32_t coloredRibbon = bullet2;
		M_SCI std::int32_t bucket = bullet3;
		M_SCI std::int32_t peach = bullet4;
		M_SCI std::int32_t rollCircle = bullet5;
		M_SCI std::int32_t palmLeafMan = bullet6;
	};

	struct PropTimeInSeconds
	{

	private:

		M_SCI std::int32_t propTimeInSeconds = 30;
		M_SCI std::int32_t mineTimeInSeconds = 60;

	public:

		//道具效果持续时间

		M_SCI std::int32_t bike = propTimeInSeconds;
		M_SCI std::int32_t amplifier = propTimeInSeconds;
		M_SCI std::int32_t jinKeLa = propTimeInSeconds;
		M_SCI std::int32_t rice = propTimeInSeconds;
		M_SCI std::int32_t negativeFeedback = propTimeInSeconds;
		M_SCI std::int32_t totem = propTimeInSeconds;
		M_SCI std::int32_t spear = propTimeInSeconds;
		M_SCI std::int32_t dirt = propTimeInSeconds;
		M_SCI std::int32_t attenuator = propTimeInSeconds;
		M_SCI std::int32_t divider = propTimeInSeconds;

		M_SCI std::int32_t sharedBike = bike;
		M_SCI std::int32_t horn = amplifier;
		M_SCI std::int32_t schoolBag = jinKeLa;
		M_SCI std::int32_t happyHotPot = rice;
		M_SCI std::int32_t shield = negativeFeedback;
		M_SCI std::int32_t clothes = totem;
		M_SCI std::int32_t javelin = spear;
		M_SCI std::int32_t puddle = dirt;
		M_SCI std::int32_t musicPlayer = attenuator;
		M_SCI std::int32_t mail = divider;

		//地雷从埋藏到消失的时间

		M_SCI std::int32_t mine = mineTimeInSeconds;
	};

	struct Game
	{
		M_SCI std::int64_t invalidGUID = (std::numeric_limits<std::int64_t>::max)();		// 无效的 GUID
		M_SCI std::int64_t noneGUID = (std::numeric_limits<std::int64_t>::min)();			// 无 GUID，正常情况下不会出现，作为保留 GUID 使用

		M_SCI std::int64_t wearClothesTimeInSeconds = 60;									// 人物下场换衣服需要的时间

		M_SCI std::int32_t thrownPropMoveSpeed = Map::numOfGridPerCell * 8;					// 扔出的道具的移动速度
	};

}

#undef MF_SCI
#undef M_SCI

#endif //!CONSTANTS_H

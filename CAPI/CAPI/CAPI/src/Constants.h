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

		M_SCI std::int32_t ordinaryBullet = bullet0;	// 6 * 1000
		M_SCI std::int32_t happyBullet = bullet1;		// 12 * 1000
		M_SCI std::int32_t coloredRibbon = bullet2;		// 3 * 1000
		M_SCI std::int32_t bucket = bullet3;			// 3 * 1000
		M_SCI std::int32_t peach = bullet4;				// 24 * 1000
		M_SCI std::int32_t rollCircle = bullet5;		// 6 * 1000
		M_SCI std::int32_t palmLeafMan = bullet6;		// 6 * 1000
	};

	// 人物的移动速度（每秒移动的坐标数）

	struct Character
	{
		struct MoveSpeed
		{
		private:

			M_SCI std::int32_t basicPlayerMoveSpeed = Map::numOfGridPerCell * 4;

		public:

			M_SCI std::int32_t job0 = basicPlayerMoveSpeed;
			M_SCI std::int32_t job1 = basicPlayerMoveSpeed;
			M_SCI std::int32_t job2 = basicPlayerMoveSpeed * 2 / 3;
			M_SCI std::int32_t job3 = basicPlayerMoveSpeed * 3 / 2;
			M_SCI std::int32_t job4 = basicPlayerMoveSpeed * 2;
			M_SCI std::int32_t job5 = basicPlayerMoveSpeed * 2;
			M_SCI std::int32_t job6 = basicPlayerMoveSpeed * 3 / 4;

			M_SCI std::int32_t OrdinaryJob = job0;		// 4000
			M_SCI std::int32_t HappyMan = job1;			// 4000
			M_SCI std::int32_t LazyGoat = job2;			// 2666
			M_SCI std::int32_t PurpleFish = job3;		// 6000
			M_SCI std::int32_t MonkeyDoctor = job4;		// 8000
			M_SCI std::int32_t EggMan = job5;			// 8000
			M_SCI std::int32_t PrincessIronFan = job6;	// 3000
		};

		// 耐污值

		struct Hp
		{
		private:
			M_SCI std::int32_t basicHP = 5000;

		public:

			M_SCI std::int32_t job0 = basicHP;
			M_SCI std::int32_t job1 = basicHP;
			M_SCI std::int32_t job2 = basicHP;
			M_SCI std::int32_t job3 = basicHP / 2;
			M_SCI std::int32_t job4 = basicHP * 2 / 3;
			M_SCI std::int32_t job5 = basicHP * 2 / 3;
			M_SCI std::int32_t job6 = basicHP * 3;

			M_SCI std::int32_t OrdinaryJob = job0;		// 5000
			M_SCI std::int32_t HappyMan = job1;			// 5000
			M_SCI std::int32_t LazyGoat = job2;			// 5000
			M_SCI std::int32_t PurpleFish = job3;		// 2500
			M_SCI std::int32_t MonkeyDoctor = job4;		// 3333
			M_SCI std::int32_t EggMan = job5;			// 3333
			M_SCI std::int32_t PrincessIronFan = job6;	// 15000
		};

		// 初始对衣物的浸染力（攻击力）

		struct AP
		{
		private:

			M_SCI std::int32_t basicAP = 1000;

		public:

			M_SCI std::int32_t job0 = basicAP;
			M_SCI std::int32_t job1 = basicAP;
			M_SCI std::int32_t job2 = basicAP * 2;
			M_SCI std::int32_t job3 = basicAP / 2;
			M_SCI std::int32_t job4 = basicAP * 3;
			M_SCI std::int32_t job5 = basicAP * 3;
			M_SCI std::int32_t job6 = basicAP;

			M_SCI std::int32_t OrdinaryJob = job0;			// 1000
			M_SCI std::int32_t HappyMan = job1;				// 1000
			M_SCI std::int32_t LazyGoat = job2;				// 2000
			M_SCI std::int32_t PurpleFish = job3;			// 500
			M_SCI std::int32_t MonkeyDoctor = job4;			// 3000
			M_SCI std::int32_t EggMan = job5;				// 3000
			M_SCI std::int32_t PrincessIronFan = job6;		// 1000
		};

		// 在自己队伍颜色的格子内回复一颗子弹所需要的时间（毫秒）

		struct CDInSeconds
		{
		private:

			M_SCI std::int32_t basicCD = 2;

		public:

			M_SCI std::int32_t job0 = basicCD;
			M_SCI std::int32_t job1 = basicCD;
			M_SCI std::int32_t job2 = basicCD * 2;
			M_SCI std::int32_t job3 = basicCD * 2;
			M_SCI std::int32_t job4 = basicCD * 2;
			M_SCI std::int32_t job5 = basicCD * 2;
			M_SCI std::int32_t job6 = basicCD;

			M_SCI std::int32_t OrdinaryJob = job0;			// 2
			M_SCI std::int32_t HappyMan = job1;				// 2
			M_SCI std::int32_t LazyGoat = job2;				// 4
			M_SCI std::int32_t PurpleFish = job3;			// 4
			M_SCI std::int32_t MonkeyDoctor = job4;			// 4
			M_SCI std::int32_t EggMan = job5;				// 4
			M_SCI std::int32_t PrincessIronFan = job6;		// 2
		};

		// 最大子弹数

		struct BulletNum
		{
		private:

			M_SCI std::int32_t basicBulletNum = 15;

		public:

			M_SCI std::int32_t job0 = basicBulletNum;
			M_SCI std::int32_t job1 = basicBulletNum;
			M_SCI std::int32_t job2 = basicBulletNum;
			M_SCI std::int32_t job3 = basicBulletNum / 2;
			M_SCI std::int32_t job4 = basicBulletNum / 3;
			M_SCI std::int32_t job5 = basicBulletNum / 3;
			M_SCI std::int32_t job6 = basicBulletNum;

			M_SCI std::int32_t OrdinaryJob = job0;			// 15
			M_SCI std::int32_t HappyMan = job1;				// 15
			M_SCI std::int32_t LazyGoat = job2;				// 15
			M_SCI std::int32_t PurpleFish = job3;			// 7
			M_SCI std::int32_t MonkeyDoctor = job4;			// 5
			M_SCI std::int32_t EggMan = job5;				// 5
			M_SCI std::int32_t PrincessIronFan = job6;		// 5
		};
	};

	struct PropTimeInSeconds
	{

	private:

		M_SCI std::int32_t propTimeInSeconds = 30;
		M_SCI std::int32_t mineTimeInSeconds = 60;

	public:

		//道具效果持续时间，目前均为 30 秒

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

		// 游戏刚开始 / 人物复活时盾的持续时间，目前为 5 秒

		M_SCI std::int32_t birthShieldTime = 5;

		//地雷从埋藏到消失的时间

		M_SCI std::int32_t mine = mineTimeInSeconds;	// 目前为 60 秒
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

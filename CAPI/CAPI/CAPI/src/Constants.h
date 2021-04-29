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
		M_SCI std::int32_t bullet4 = basicBulletMoveSpeed * 3;
		M_SCI std::int32_t bullet5 = basicBulletMoveSpeed;
		M_SCI std::int32_t bullet6 = basicBulletMoveSpeed;

		M_SCI std::int32_t ordinaryBullet = bullet0;	// 6 * 1000
		M_SCI std::int32_t happyBullet = bullet1;		// 12 * 1000
		M_SCI std::int32_t coloredRibbon = bullet2;		// 3 * 1000
		M_SCI std::int32_t bucket = bullet3;			// 3 * 1000
		M_SCI std::int32_t peach = bullet4;				// 18 * 1000
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
			M_SCI std::int32_t job2 = basicPlayerMoveSpeed / 3;
			M_SCI std::int32_t job3 = basicPlayerMoveSpeed * 3 / 2;
			M_SCI std::int32_t job4 = basicPlayerMoveSpeed * 2;
			M_SCI std::int32_t job5 = basicPlayerMoveSpeed * 2;
			M_SCI std::int32_t job6 = basicPlayerMoveSpeed;

			M_SCI std::int32_t OrdinaryJob = job0;		// 4000
			M_SCI std::int32_t HappyMan = job1;			// 4000
			M_SCI std::int32_t LazyGoat = job2;			// 1333
			M_SCI std::int32_t PurpleFish = job3;		// 6000
			M_SCI std::int32_t MonkeyDoctor = job4;		// 8000
			M_SCI std::int32_t EggMan = job5;			// 8000
			M_SCI std::int32_t PrincessIronFan = job6;	// 4000
		};

		// 耐污值

		struct Hp
		{
		private:
			M_SCI std::int32_t basicHP = 6000;

		public:

			M_SCI std::int32_t job0 = basicHP;
			M_SCI std::int32_t job1 = basicHP * 4 / 3;
			M_SCI std::int32_t job2 = basicHP;
			M_SCI std::int32_t job3 = basicHP * 2 / 5;
			M_SCI std::int32_t job4 = basicHP * 2 / 3;
			M_SCI std::int32_t job5 = basicHP * 2 / 3;
			M_SCI std::int32_t job6 = basicHP * 2;

			M_SCI std::int32_t OrdinaryJob = job0;		// 6000
			M_SCI std::int32_t HappyMan = job1;			// 8000
			M_SCI std::int32_t LazyGoat = job2;			// 6000
			M_SCI std::int32_t PurpleFish = job3;		// 2400
			M_SCI std::int32_t MonkeyDoctor = job4;		// 4000
			M_SCI std::int32_t EggMan = job5;			// 4000
			M_SCI std::int32_t PrincessIronFan = job6;	// 12000
		};

		// 初始对衣物的浸染力（攻击力）

		struct AP
		{
		private:

			M_SCI std::int32_t basicAP = 1000;

		public:

			M_SCI std::int32_t job0 = basicAP;
			M_SCI std::int32_t job1 = basicAP * 3 / 4;
			M_SCI std::int32_t job2 = basicAP * 5 / 4;
			M_SCI std::int32_t job3 = basicAP * 3 / 8;
			M_SCI std::int32_t job4 = basicAP * 7 / 2;
			M_SCI std::int32_t job5 = basicAP * 4;
			M_SCI std::int32_t job6 = basicAP / 2;

			M_SCI std::int32_t OrdinaryJob = job0;			// 1000
			M_SCI std::int32_t HappyMan = job1;				// 750
			M_SCI std::int32_t LazyGoat = job2;				// 1250
			M_SCI std::int32_t PurpleFish = job3;			// 375
			M_SCI std::int32_t MonkeyDoctor = job4;			// 3500
			M_SCI std::int32_t EggMan = job5;				// 4000
			M_SCI std::int32_t PrincessIronFan = job6;		// 500
		};

		// 在自己队伍颜色的格子内回复一颗子弹所需要的时间（毫秒）

		struct CDInSeconds
		{
		private:

			M_SCI std::int32_t basicCD = 1;

		public:

			M_SCI std::int32_t job0 = basicCD;
			M_SCI std::int32_t job1 = basicCD;
			M_SCI std::int32_t job2 = basicCD * 2;
			M_SCI std::int32_t job3 = basicCD * 3;
			M_SCI std::int32_t job4 = basicCD * 4;
			M_SCI std::int32_t job5 = basicCD * 2;
			M_SCI std::int32_t job6 = basicCD;

			M_SCI std::int32_t OrdinaryJob = job0;			// 1
			M_SCI std::int32_t HappyMan = job1;				// 1
			M_SCI std::int32_t LazyGoat = job2;				// 2
			M_SCI std::int32_t PurpleFish = job3;			// 3
			M_SCI std::int32_t MonkeyDoctor = job4;			// 4
			M_SCI std::int32_t EggMan = job5;				// 2
			M_SCI std::int32_t PrincessIronFan = job6;		// 1
		};

		// 最大子弹数

		struct BulletNum
		{
		private:

			M_SCI std::int32_t basicBulletNum = 12;

		public:

			M_SCI std::int32_t job0 = basicBulletNum;
			M_SCI std::int32_t job1 = basicBulletNum;
			M_SCI std::int32_t job2 = basicBulletNum;
			M_SCI std::int32_t job3 = basicBulletNum / 2;
			M_SCI std::int32_t job4 = basicBulletNum / 4;
			M_SCI std::int32_t job5 = basicBulletNum / 3;
			M_SCI std::int32_t job6 = basicBulletNum;

			M_SCI std::int32_t OrdinaryJob = job0;			// 12
			M_SCI std::int32_t HappyMan = job1;				// 12
			M_SCI std::int32_t LazyGoat = job2;				// 12
			M_SCI std::int32_t PurpleFish = job3;			// 6
			M_SCI std::int32_t MonkeyDoctor = job4;			// 3
			M_SCI std::int32_t EggMan = job5;				// 4
			M_SCI std::int32_t PrincessIronFan = job6;		// 12
		};
	};

	struct Prop
	{
		struct TimeInSeconds
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

		// 道具效果

		struct Effect
		{
			M_SCI double sharedBike = 1.5;		// 使人物移动速度增加为原来的 1.5 倍
			M_SCI double horn = 2.0;			// 使一定时间内彩粉对其他人物衣服的浸染力（即攻击力）增加为原来的 2.0 倍
			M_SCI double schoolBag = 0.25;		// 使在自己颜色的格子里回复子弹所需时间变为原来的 0.25 倍
			M_SCI std::int32_t happyHotPot = 2500;	// 回复 HP 2500 点

			M_SCI double puddle = 1.0 / 1.5;	// 同 sharedBike（除数值不同外。下同）
			M_SCI double musicPlayer = 0.5;		// 同 horn
			M_SCI double mail = 4.0;			// 同 schoolBag
		};
	};

	struct Game
	{
		M_SCI std::int64_t invalidGUID = (std::numeric_limits<std::int64_t>::max)();		// 无效的 GUID
		M_SCI std::int64_t noneGUID = (std::numeric_limits<std::int64_t>::min)();			// 无 GUID，正常情况下不会出现，作为保留 GUID 使用

		M_SCI std::int64_t wearClothesTimeInSeconds = 30;									// 人物下场换衣服需要的时间
		M_SCI std::int64_t produceOnePropTimeInSeconds = 10;								// 游戏每产生两个道具之间的时间间隔
		M_SCI std::int32_t thrownPropMoveSpeed = Map::numOfGridPerCell * 8;					// 扔出的道具的移动速度
	};

}

#undef MF_SCI
#undef M_SCI

#endif //!CONSTANTS_H

#pragma once

#ifndef STRUCTURES_H

#define STRUCTURES_H

#include <memory>
#include <cstdint>
#include <array>
#include <vector>

#define _COLOR_MAP_BY_HASHING_

#ifdef _COLOR_MAP_BY_HASHING_
#include<unordered_map>
#endif

namespace THUAI4
{

	enum class PropType : unsigned char
	{
		Null = 0,
		Bike = 1,
		Amplifier = 2,
		JinKeLa = 3,
		Rice = 4,
		NegativeFeedback = 5,
		Totem = 6,
		Phaser = 7,
		Dirt = 8,
		Attenuator = 9,
		Divider = 10
	};

	enum class ShapeType : unsigned char
	{
		Circle = 0,
		Square = 1
	};

	enum class JobType : unsigned char
	{
		Job0 = 0,
		Job1 = 1,
		Job2 = 2,
		Job3 = 3,
		Job4 = 4,
		Job5 = 5,
		Job6 = 6,
	};

	enum class BulletType : unsigned char
	{
		Bullet0 = 0,
		Bullet1 = 1,
		Bullet2 = 2,
		Bullet3 = 3,
		Bullet4 = 4,
		Bullet5 = 5,
		Bullet6 = 6
	};

	enum class ColorType : unsigned char
	{
		None = 0,
		Color1 = 1,
		Color2 = 2,
		Color3 = 3,
		Color4 = 4,
		Invisible = 5
	};

	struct Character 
	{
		bool isMoving;
		bool isDying;
		ShapeType shapeType : 1;
		BulletType bulletType : 4;
		PropType propType : 4;
		JobType jobType : 4;
		uint16_t lifeNum;
		uint16_t teamID;
		uint16_t radius;
		uint16_t maxBulletNum;
		uint16_t bulletNum;

		uint32_t x;
		uint32_t y;
		uint32_t moveSpeed;
		uint32_t ap;
		uint32_t maxHp;
		uint32_t hp;
		uint32_t CD;
		uint64_t guid;
		double facingDirection;
	};

	struct Wall 
	{
		ShapeType shapeType : 1;
		uint16_t radius;
		uint32_t x;
		uint32_t y;
		uint64_t guid;
	};

	struct Prop 
	{
		bool isMoving;
		bool isLaid;
		ShapeType shapeType : 1;
		PropType propType : 4;
		uint16_t radius;
		uint32_t x;
		uint32_t y;
		uint32_t moveSpeed;
		uint64_t guid;
		double facingDirection;
	};

	struct Bullet 
	{
		bool isMoving;
		ShapeType shapeType : 1;
		BulletType bulletType : 4;
		uint16_t radius;
		uint16_t teamID;
		uint32_t x;
		uint32_t y;
		uint32_t moveSpeed;
		uint32_t ap;
		uint64_t guid;
		double facingDirection;
	};

	struct BirthPoint 
	{
		ShapeType shapeType;
		uint16_t teamID;
		uint16_t radius;
		uint32_t x;
		uint32_t y;
		uint64_t guid;
	};


	struct State 
	{
		constexpr static uint32_t nTeams = 2;
		constexpr static uint32_t nPlayers = 4;
		constexpr static uint32_t nCells = 50;
		ColorType selfTeamColor;
		uint32_t teamScore;
		std::vector<std::shared_ptr<Character>> characters;
		std::vector<std::shared_ptr<Wall>> walls;
		std::vector<std::shared_ptr<Prop>> props;
		std::vector<std::shared_ptr<Bullet>> bullets;
		std::vector<std::shared_ptr<BirthPoint>> birthpoints;
		std::array<std::array<uint32_t, nPlayers>, nTeams> playerGUIDs;
		std::shared_ptr<Character> self;
#ifdef _COLOR_MAP_BY_HASHING_
		std::unordered_map<uint32_t, ColorType> cellColors;
#else
		std::array<std::array<ColorType, nCells>, nCells> cellColors;
#endif
	};
}

#endif //!STRUCTURES_H

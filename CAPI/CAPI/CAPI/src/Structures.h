#pragma once

#ifndef STRUCTURES_H

#define STRUCTURES_H

#include <cstdint>
#include <array>

namespace THUAI4
{

	enum class PropType : unsigned char    //道具类型
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

	enum class ShapeType : unsigned char    //物体形状
	{
		Circle = 0,
		Square = 1
	};

	enum class JobType : unsigned char    //玩家职业
	{
		Job0 = 0,
		Job1 = 1,
		Job2 = 2,
		Job3 = 3,
		Job4 = 4,
		Job5 = 5,
		Job6 = 6,
	};

	enum class BulletType : unsigned char    //子弹种类
	{
		Bullet0 = 0,
		Bullet1 = 1,
		Bullet2 = 2,
		Bullet3 = 3,
		Bullet4 = 4,
		Bullet5 = 5,
		Bullet6 = 6
	};

	enum class ColorType : unsigned char    //颜色种类
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
		bool isMoving;  //是否正在移动
		bool isDying;  //是否正在复活中
		ShapeType shapeType : 2;  //形状（圆）
		BulletType bulletType : 4;  //子弹种类
		PropType propType : 4;  //所拥有道具类型
		JobType jobType : 4;  //职业类型
		uint16_t lifeNum;  //第几条命
		uint16_t teamID;  //队伍ID
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint16_t maxBulletNum;  //弹夹最大容量
		uint16_t bulletNum;  //当前子弹数量

		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		uint32_t moveSpeed;  //移动速度
		uint32_t ap;  //攻击力
		uint32_t maxHp;  //最大生命值
		uint32_t hp;  //当前声明值
		uint32_t CD;  //设计冷却时间
		int64_t guid;  //Global Unique Identifier
		double facingDirection;  //移动朝向
	};

	struct Wall
	{
		ShapeType shapeType : 2;  //形状（正方形）
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		int64_t guid;  //Global Unique Identifier
	};

	struct Prop
	{
		bool isMoving;  //是否正在移动
		bool isLaid;  //是否已被放置（选手能看到的一定没被放置）
		ShapeType shapeType : 2;  //形状
		PropType propType : 4;  //道具种类
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		uint32_t moveSpeed;  //移动速度
		int64_t guid;  //Global Unique Identifier
		double facingDirection;  //移动朝向
	};

	struct Bullet
	{
		bool isMoving;  //是否在移动（是）
		ShapeType shapeType : 2;  //形状（圆）
		BulletType bulletType : 4;  //子弹种类
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint16_t teamID;  //所属队伍
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		uint32_t moveSpeed;  //移动速度
		uint32_t ap;  //攻击力
		int64_t guid;  //Global Unique Identifier
		double facingDirection;  //移动朝向
	};

	struct BirthPoint
	{
		ShapeType shapeType;  //形状（圆）
		uint16_t teamID;  //队伍ID
		uint16_t radius;  //圆形物体的半径或正方形内切圆半径
		uint32_t x;  //x坐标
		uint32_t y;  //y坐标
		int64_t guid;  //Global Unique Identifier
	};

}

#endif //!STRUCTURES_H

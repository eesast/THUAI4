#pragma once

#ifndef BASE_H

#define BASE_H

#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "Structures.h"

struct StateConstant
{
	constexpr static inline int nTeams = 2;
	constexpr static inline int nPlayers = 4;
	constexpr static inline int nCells = 50;
};

class GameApi
{
public:
	virtual bool MovePlayer(uint32_t timeInMilliseconds, double angle) = 0;
	virtual bool MoveRight(uint32_t timeInMilliseconds) = 0;
	virtual bool MoveUp(uint32_t timeInMilliseconds) = 0;
	virtual bool MoveLeft(uint32_t timeInMilliseconds) = 0;
	virtual bool MoveDown(uint32_t timeInMilliseconds) = 0;
	virtual bool Use() = 0;
	virtual bool Pick(THUAI4::PropType propType) = 0;
	virtual bool Throw(uint32_t timeInMilliseconds, double angle) = 0;
	virtual bool Attack(uint32_t timeInMilliseconds, double angle) = 0;
	virtual bool Send(int toPlayerID, std::string message) = 0;

	virtual bool Wait() = 0;//0游戏结束直接返回 1正常等

	//选手可获取的信息
	[[nodiscard]] virtual int GetCounterOfFrames() = 0;//约定-1表示游戏结束
	[[nodiscard]] virtual bool MessageAvailable() = 0;
	[[nodiscard]] virtual bool TryGetMessage(std::string&) = 0;

	[[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI4::Character>> GetCharacters() const = 0;
	[[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI4::Wall>> GetWalls() const = 0;
	[[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI4::Prop>> GetProps() const = 0;
	[[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI4::Bullet>> GetBullets() const = 0;
	[[nodiscard]] virtual std::vector<std::shared_ptr<const THUAI4::BirthPoint>> GetBirthPoints() const = 0;
	[[nodiscard]] virtual std::shared_ptr<const THUAI4::Character> GetSelfInfo() const = 0;

	[[nodiscard]] virtual THUAI4::ColorType GetSelfTeamColor() const = 0;
	[[nodiscard]] virtual uint32_t GetTeamScore() const = 0;
	[[nodiscard]] virtual const std::vector<std::vector<int64_t>> GetPlayerGUIDs() const = 0;
	[[nodiscard]] virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const = 0;

};

class AIBase
{
public:
	virtual void play(GameApi& g) = 0;
};
using CreateAIFunc = std::unique_ptr<AIBase>(*)();

int thuai4_main(int argc, char** argv, CreateAIFunc AIBuilder);

class ID
{
private:
	static inline int playerID = 0;
	static inline int teamID = 0;

public:
	[[nodiscard]] static int GetPlayerID() { return playerID; }
	[[nodiscard]] static int GetTeamID() { return teamID; }

	friend int thuai4_main(int argc, char** argv, CreateAIFunc AIBuilder);
};

#endif // !BASE_H

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
	virtual void MovePlayer(uint32_t timeInMilliseconds, double angle) = 0;
	virtual void MoveRight(uint32_t timeInMilliseconds) = 0;
	virtual void MoveUp(uint32_t timeInMilliseconds) = 0;
	virtual void MoveLeft(uint32_t timeInMilliseconds) = 0;
	virtual void MoveDown(uint32_t timeInMilliseconds) = 0;
	virtual void Use() = 0;
	virtual void Pick(THUAI4::PropType propType) = 0;
	virtual void Throw(uint32_t timeInMilliseconds, double angle) = 0;
	virtual void Attack(uint32_t timeInMilliseconds, double angle) = 0;
	virtual void Send(int toPlayerID, std::string message) = 0;

	//选手可获取的信息
	virtual bool MessageAvailable() = 0;
	virtual bool TryGetMessage(std::string &) = 0;

	virtual std::vector<std::shared_ptr<const THUAI4::Character>> GetCharacters() const = 0;
	virtual std::vector<std::shared_ptr<const THUAI4::Wall>> GetWalls() const = 0;
	virtual std::vector<std::shared_ptr<const THUAI4::Prop>> GetProps() const = 0;
	virtual std::vector<std::shared_ptr<const THUAI4::Bullet>> GetBullets() const = 0;
	virtual std::vector<std::shared_ptr<const THUAI4::BirthPoint>> GetBirthPoints() const = 0;
	virtual std::shared_ptr<const THUAI4::Character> GetSelfInfo() const = 0;

	virtual THUAI4::ColorType GetSelfTeamColor() const = 0;
	virtual uint32_t GetTeamScore() const = 0;
	virtual const std::array<std::array<int64_t, StateConstant::nPlayers>, StateConstant::nTeams> &GetPlayerGUIDs() const = 0;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const = 0;
};

class AIBase
{
public:
	virtual void play(GameApi &g) = 0;
};
using CreateAIFunc = std::unique_ptr<AIBase> (*)();

int thuai4_main(int argc, char **argv, CreateAIFunc AIBuilder);

class ID
{
private:
	static inline int playerID = 0;
	static inline int teamID = 0;

public:
	static int GetPlayerID() { return playerID; }
	static int GetTeamID() { return teamID; }

	friend int thuai4_main(int argc, char **argv, CreateAIFunc AIBuilder);
};

#endif // !BASE_H

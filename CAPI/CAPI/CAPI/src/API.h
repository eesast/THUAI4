#pragma once

#ifndef API_H

#define API_H

#include <mutex>
#include <string>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <functional>
#include <cstdint>
#include <unordered_map>
#include "concurrent_queue.hpp"

#include "proto/Message2Server.pb.h"
#include "Structures.h"
#include "Constants.h"
#include "Base.h"
#define _COLOR_MAP_BY_HASHING_

struct State
{
	THUAI4::ColorType selfTeamColor;
	uint32_t teamScore;
	std::vector<std::shared_ptr<THUAI4::Character>> characters;
	std::vector<std::shared_ptr<THUAI4::Wall>> walls;
	std::vector<std::shared_ptr<THUAI4::Prop>> props;
	std::vector<std::shared_ptr<THUAI4::Bullet>> bullets;
	std::vector<std::shared_ptr<THUAI4::BirthPoint>> birthpoints;
	std::array<std::array<int64_t, StateConstant::nPlayers>, StateConstant::nTeams> playerGUIDs;
	std::shared_ptr<THUAI4::Character> self;
#ifdef _COLOR_MAP_BY_HASHING_
	std::unordered_map<uint32_t, THUAI4::ColorType> cellColors;
#else
	std::array<std::array<ColorType, nCells>, nCells> cellColors;
#endif
};

struct LogicInterface : public GameApi
{
protected:
	const std::function<void(Protobuf::MessageToServer &)> SendMessageWrapper; //加入ID放到这个函数里了
	const std::function<bool()> Empty;
	const std::function<bool(std::string &)> TryPop;
	const State *&pState;

public:
	LogicInterface(std::function<void(Protobuf::MessageToServer &)> sm,
				   std::function<bool()> e, std::function<bool(std::string &)> tp,
				   const State *&pS) : SendMessageWrapper(sm), Empty(e), TryPop(tp), pState(pS) {}
	virtual void StartTimer() = 0;
	virtual void EndTimer() = 0;
};

template <bool asyn> //如果为真，仅API函数调用与state更新互斥 否则play()函数调用期间state更新都阻塞
class API final : public LogicInterface
{
private:
	virtual void StartTimer() {}
	virtual void EndTimer() {}
	std::mutex &mtx_state; //并不总是需要，但……忍了吧
	std::function<void()> TryUpDate;

public:
	API(std::function<void(Protobuf::MessageToServer &)> sm,
		std::function<bool()> e, std::function<bool(std::string &)> tp,
		const State *&pS, std::mutex &mtx_game, std::function<void()>);
	virtual void MovePlayer(uint32_t timeInMilliseconds, double angle);
	virtual void MoveRight(uint32_t timeInMilliseconds);
	virtual void MoveUp(uint32_t timeInMilliseconds);
	virtual void MoveLeft(uint32_t timeInMilliseconds);
	virtual void MoveDown(uint32_t timeInMilliseconds);
	virtual void Use();
	virtual void Pick(THUAI4::PropType propType);
	virtual void Throw(uint32_t timeInMilliseconds, double angle);
	virtual void Attack(uint32_t timeInMilliseconds, double angle);
	virtual void Send(int toPlayerID, std::string message);

	//Information the player can get
	virtual bool MessageAvailable();
	virtual bool TryGetMessage(std::string &);

	virtual std::vector<const THUAI4::Character *> GetCharacterPtrs() const;
	virtual std::vector<const THUAI4::Wall *> GetWallPtrs() const;
	virtual std::vector<const THUAI4::Prop *> GetPropPtrs() const;
	virtual std::vector<const THUAI4::Bullet *> GetBulletPtrs() const;
	virtual std::vector<const THUAI4::BirthPoint *> GetBirthPointPtrs() const;
	virtual const THUAI4::Character &GetSelfInfoRef() const;

	virtual std::vector<THUAI4::Character> GetCharacters() const;
	virtual std::vector<THUAI4::Wall> GetWalls() const;
	virtual std::vector<THUAI4::Prop> GetProps() const;
	virtual std::vector<THUAI4::Bullet> GetBullets() const;
	virtual std::vector<THUAI4::BirthPoint> GetBirthPoints() const;
	virtual THUAI4::Character GetSelfInfo() const;

	virtual THUAI4::ColorType GetSelfTeamColor() const;
	virtual uint32_t GetTeamScore() const;
	virtual const std::array<std::array<int64_t, StateConstant::nPlayers>, StateConstant::nTeams> &GetPlayerGUIDs() const override;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};

template <bool asyn>
class DebugApi final : public LogicInterface
{
private:
	bool ExamineValidity;
	std::ostream &OutStream;
	std::chrono::system_clock::time_point StartPoint;
	bool CanPick(THUAI4::PropType propType);
	std::map<THUAI4::PropType, std::string> dict{
		{THUAI4::PropType::Amplifier, "Amplifier"},
		{THUAI4::PropType::Attenuator, "Attenuator"},
		{THUAI4::PropType::Bike, "Bike"},
		{THUAI4::PropType::Dirt, "Dirt"},
		{THUAI4::PropType::Divider, "Divider"},
		{THUAI4::PropType::JinKeLa, "JinKeLa"},
		{THUAI4::PropType::NegativeFeedback, "NegativeFeedback"},
		{THUAI4::PropType::Null, "Null"},
		{THUAI4::PropType::Phaser, "Phaser"},
		{THUAI4::PropType::Rice, "Rice"},
		{THUAI4::PropType::Totem, "Totem"}};
	virtual void StartTimer();
	virtual void EndTimer();
	std::mutex &mtx_state;
	std::function<void()> TryUpDate;

public:
	DebugApi(std::function<void(Protobuf::MessageToServer &)> sm,
			 std::function<bool()> e, std::function<bool(std::string &)> tp,
			 const State *&pS, std::mutex &mtx_game, std::function<void()>, bool ev = false,
			 std::ostream &out = std::cout);
	virtual void MovePlayer(uint32_t timeInMilliseconds, double angle);
	virtual void MoveRight(uint32_t timeInMilliseconds);
	virtual void MoveUp(uint32_t timeInMilliseconds);
	virtual void MoveLeft(uint32_t timeInMilliseconds);
	virtual void MoveDown(uint32_t timeInMilliseconds);
	virtual void Use();
	virtual void Pick(THUAI4::PropType propType);
	virtual void Throw(uint32_t timeInMilliseconds, double angle);
	virtual void Attack(uint32_t timeInMilliseconds, double angle);
	virtual void Send(int toPlayerID, std::string message);

	//Information the player can get
	virtual bool MessageAvailable();
	virtual bool TryGetMessage(std::string &);

	virtual std::vector<const THUAI4::Character *> GetCharacterPtrs() const;
	virtual std::vector<const THUAI4::Wall *> GetWallPtrs() const;
	virtual std::vector<const THUAI4::Prop *> GetPropPtrs() const;
	virtual std::vector<const THUAI4::Bullet *> GetBulletPtrs() const;
	virtual std::vector<const THUAI4::BirthPoint *> GetBirthPointPtrs() const;
	virtual const THUAI4::Character &GetSelfInfoRef() const;

	virtual std::vector<THUAI4::Character> GetCharacters() const;
	virtual std::vector<THUAI4::Wall> GetWalls() const;
	virtual std::vector<THUAI4::Prop> GetProps() const;
	virtual std::vector<THUAI4::Bullet> GetBullets() const;
	virtual std::vector<THUAI4::BirthPoint> GetBirthPoints() const;
	virtual THUAI4::Character GetSelfInfo() const;

	virtual THUAI4::ColorType GetSelfTeamColor() const;
	virtual uint32_t GetTeamScore() const;
	virtual const std::array<std::array<int64_t, StateConstant::nPlayers>, StateConstant::nTeams> &GetPlayerGUIDs() const override;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};
inline bool CellColorVisible(int32_t x, int32_t y, int32_t CellX, int32_t CellY)
{
	int32_t centerX = CellX * Constants::Map::numOfGridPerCell + (Constants::Map::numOfGridPerCell >> 1);
	int32_t centerY = CellY * Constants::Map::numOfGridPerCell + (Constants::Map::numOfGridPerCell >> 1);
	int32_t dx = std::abs(centerX - x);
	int32_t dy = std::abs(centerY - y);
	int32_t D = (Constants::Map::numOfGridPerCell >> 1) + Constants::Map::sightRadius;
	return dx <= D && dy <= D;
}

#endif // !API_H

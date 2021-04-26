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
	static std::vector<std::vector<int64_t>> playerGUIDs;
	std::shared_ptr<THUAI4::Character> self;
#ifdef _COLOR_MAP_BY_HASHING_
	std::unordered_map<uint32_t, THUAI4::ColorType> cellColors;
#else
	std::array<std::array<THUAI4::ColorType, StateConstant::nCells>, StateConstant::nCells> cellColors;
#endif
};

struct LogicInterface : public GameApi
{
protected:
	const std::function<bool(Protobuf::MessageToServer&)> SendMessageWrapper; //加入ID放到这个函数里了
	const std::function<bool()> Empty;
	const std::function<bool(std::string&)> TryPop;
	const std::function<int()> GetCounter;
	const std::function<void()> _Wait;
	const State*& pState;

public:
	LogicInterface(std::function<bool(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp, std::function<int()> gc,
		const State*& pS, std::function<void()> w) : SendMessageWrapper(sm), Empty(e), TryPop(tp), GetCounter(gc), pState(pS), _Wait(w) {}
	virtual void StartTimer() = 0;
	virtual void EndTimer() = 0;
};

template <bool>
struct Members
{
public:
	Members(std::mutex& mtx_state, std::function<void()> f) {}
};

template <>
struct Members<true>
{
public:
	Members(std::mutex& mtx_state, std::function<void()> f) : mtx_state(mtx_state), TryUpDate(f) {}

protected:
	std::mutex& mtx_state;
	const std::function<void()> TryUpDate;
};

template <bool asyn> //如果为真，仅API函数调用与state更新互斥 否则play()函数调用期间state更新都阻塞
class API final : public LogicInterface, Members<asyn>
{
private:
	virtual void StartTimer() {}
	virtual void EndTimer() {}

public:
	API(std::function<bool(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp, std::function<int()> gc,
		const State*& pS, std::mutex& mtx_state, std::function<void()> tu, std::function<void()> w);
	virtual bool MovePlayer(uint32_t timeInMilliseconds, double angle);
	virtual bool MoveRight(uint32_t timeInMilliseconds);
	virtual bool MoveUp(uint32_t timeInMilliseconds);
	virtual bool MoveLeft(uint32_t timeInMilliseconds);
	virtual bool MoveDown(uint32_t timeInMilliseconds);
	virtual bool Use();
	virtual bool Pick(THUAI4::PropType propType);
	virtual bool Throw(uint32_t timeInMilliseconds, double angle);
	virtual bool Attack(uint32_t timeInMilliseconds, double angle);
	virtual bool Send(int toPlayerID, std::string message);

	virtual bool Wait();

	//Information the player can get
	virtual int GetCounterOfFrames();
	virtual bool MessageAvailable();
	virtual bool TryGetMessage(std::string&);

	virtual std::vector<std::shared_ptr<const THUAI4::Character>> GetCharacters() const;
	virtual std::vector<std::shared_ptr<const THUAI4::Wall>> GetWalls() const;
	virtual std::vector<std::shared_ptr<const THUAI4::Prop>> GetProps() const;
	virtual std::vector<std::shared_ptr<const THUAI4::Bullet>> GetBullets() const;
	virtual std::vector<std::shared_ptr<const THUAI4::BirthPoint>> GetBirthPoints() const;
	virtual std::shared_ptr<const THUAI4::Character> GetSelfInfo() const;

	virtual THUAI4::ColorType GetSelfTeamColor() const;
	virtual uint32_t GetTeamScore() const;
	virtual const std::vector<std::vector<int64_t>> GetPlayerGUIDs() const override;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};


template class API<true>;
template class API<false>;

template <bool asyn>
class DebugApi final : public LogicInterface, Members<asyn>
{
private:
	bool ExamineValidity;
	std::ostream& OutStream;
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
		{THUAI4::PropType::Totem, "Totem"} };
	virtual void StartTimer();
	virtual void EndTimer();

public:
	DebugApi(std::function<bool(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp, std::function<int()> gc,
		const State*& pS, std::mutex& mtx_state, std::function<void()> tu, std::function<void()> w, bool ev = false,
		std::ostream& out = std::cout);
	virtual bool MovePlayer(uint32_t timeInMilliseconds, double angle);
	virtual bool MoveRight(uint32_t timeInMilliseconds);
	virtual bool MoveUp(uint32_t timeInMilliseconds);
	virtual bool MoveLeft(uint32_t timeInMilliseconds);
	virtual bool MoveDown(uint32_t timeInMilliseconds);
	virtual bool Use();
	virtual bool Pick(THUAI4::PropType propType);
	virtual bool Throw(uint32_t timeInMilliseconds, double angle);
	virtual bool Attack(uint32_t timeInMilliseconds, double angle);
	virtual bool Send(int toPlayerID, std::string message);

	virtual bool Wait();

	//Information the player can get
	virtual int GetCounterOfFrames();
	virtual bool MessageAvailable();
	virtual bool TryGetMessage(std::string&);

	virtual std::vector<std::shared_ptr<const THUAI4::Character>> GetCharacters() const;
	virtual std::vector<std::shared_ptr<const THUAI4::Wall>> GetWalls() const;
	virtual std::vector<std::shared_ptr<const THUAI4::Prop>> GetProps() const;
	virtual std::vector<std::shared_ptr<const THUAI4::Bullet>> GetBullets() const;
	virtual std::vector<std::shared_ptr<const THUAI4::BirthPoint>> GetBirthPoints() const;
	virtual std::shared_ptr<const THUAI4::Character> GetSelfInfo() const;

	virtual THUAI4::ColorType GetSelfTeamColor() const;
	virtual uint32_t GetTeamScore() const;
	virtual const std::vector<std::vector<int64_t>> GetPlayerGUIDs() const override;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};

template class DebugApi<true>;
template class DebugApi<false>;

inline uint64_t distance_squared(int32_t x, int32_t y)
{
	return x * x + y * y;
}
inline bool CellColorVisible(int32_t x, int32_t y, int32_t CellX, int32_t CellY)
{
	constexpr int32_t half = Constants::Map::numOfGridPerCell >> 1;
	int32_t centerX = (CellX << 10) - (CellX << 4) - (CellX << 3) + half;
	int32_t centerY = (CellY << 10) - (CellY << 4) - (CellY << 3) + half;
	int32_t dx = (std::max)(std::abs(centerX - x) - half, 0);
	int32_t dy = (std::max)(std::abs(centerY - y) - half, 0);
	return Constants::Map::sightRadiusSquared >= distance_squared(dx, dy);
}


#endif // !API_H

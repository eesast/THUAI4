#pragma once

#include<string>
#include"proto/Message2Server.pb.h"
#include<HPSocket/HPSocket.h>
#include<HPSocket/SocketInterface.h>
#include<functional>
#include"Structures.h"
#include<cstdint>
#include"Base.h"
#include"concurrent_queue.hpp"

#define _COLOR_MAP_BY_HASHING_

class Logic;

struct State
{
    constexpr static uint32_t nTeams = Constants::numOfTeam;
    constexpr static uint32_t nPlayers = Constants::numOfPlayer;
    constexpr static uint32_t nCells = 50;
    THUAI4::ColorType selfTeamColor;
    uint32_t teamScore;
    std::vector<std::shared_ptr<THUAI4::Character>> characters;
    std::vector<std::shared_ptr<THUAI4::Wall>> walls;
    std::vector<std::shared_ptr<THUAI4::Prop>> props;
    std::vector<std::shared_ptr<THUAI4::Bullet>> bullets;
    std::vector<std::shared_ptr<THUAI4::BirthPoint>> birthpoints;
    std::array<std::array<uint32_t, nPlayers>, nTeams> playerGUIDs;
    std::shared_ptr<THUAI4::Character> self;
#ifdef _COLOR_MAP_BY_HASHING_
    std::unordered_map<uint32_t, THUAI4::ColorType> cellColors;
#else
    std::array<std::array<ColorType, nCells>, nCells> cellColors;
#endif
};

class API:public GameApi
{
    const int32_t playerID;
    const int32_t teamID;
    State* const pState;
    const std::function<void(const Protobuf::MessageToServer&)> SendMessage;
    concurrency::concurrent_queue<std::string> MessageStorage;

public:

	API(std::function<void(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp,
		const THUAI4::State*& pS);
	virtual void MovePlayer(int timeInMilliseconds, double angle);
	virtual void MoveRight(int timeInMilliseconds);
	virtual void MoveUp(int timeInMilliseconds);
	virtual void MoveLeft(int timeInMilliseconds);
	virtual void MoveDown(int timeInMilliseconds);
	virtual void Use();
	virtual void Pick(THUAI4::PropType propType);
	virtual void Throw(int timeInMilliseconds, double angle);
	virtual void Attack(int timeInMilliseconds, double angle);
	virtual void Send(int toPlayerID, std::string message);

	//Information the player can get
	virtual bool MessageAvailable();
	virtual bool TryGetMessage(std::string&);

	virtual std::vector<const THUAI4::Character*> GetCharacters() const;
	virtual std::vector<const THUAI4::Wall*> GetWalls() const;
	virtual std::vector<const THUAI4::Prop*> GetProps() const;
	virtual std::vector<const THUAI4::Bullet*> GetBullets() const;
	virtual std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const;
	virtual const THUAI4::Character& GetSelfInfo() const;
	virtual THUAI4::ColorType GetSelfTeamColor() const;
	virtual uint32_t GetTeamScore() const;
	virtual const std::array<std::array<uint32_t, State::nPlayers>, State::nTeams>& GetPlayerGUIDs() const;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};



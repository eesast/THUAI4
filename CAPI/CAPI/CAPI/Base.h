#pragma once
#include<cstdint>
#include"Structures.h"
#include"proto/Message2Server.pb.h"

class GameApi {
public:
	virtual void MovePlayer(int timeInMilliseconds, double angle) = 0;
	virtual void MoveRight(int timeInMilliseconds) = 0;;
	virtual void MoveUp(int timeInMilliseconds) = 0;
	virtual void MoveLeft(int timeInMilliseconds) = 0;
	virtual void MoveDown(int timeInMilliseconds) = 0;
	virtual void Use() = 0;
	virtual void Pick(Protobuf::PropType propType) = 0;
	virtual void Throw(int timeInMilliseconds, double angle) = 0;
	virtual void Attack(int timeInMilliseconds, double angle) = 0;
	virtual void Send(int toPlayerID, std::string message) = 0;

	//选手可获取的信息
	virtual bool MessageAvailable() = 0;
	virtual bool TryGetMessage(std::string&) = 0;
	virtual std::vector<const THUAI4::Character*> GetCharacters() const = 0;
	virtual std::vector<const THUAI4::Wall*> GetWalls() const = 0;
	virtual std::vector<const THUAI4::Prop*> GetProps() const = 0;
	virtual std::vector<const THUAI4::Bullet*> GetBullets() const = 0;
	virtual std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const = 0;
	virtual const THUAI4::Character& GetSelfInfo() const = 0;
	virtual THUAI4::ColorType GetSelfTeamColor() const = 0;
	virtual uint32_t GetTeamScore() const = 0;
	virtual const std::array<std::array<uint32_t, THUAI4::State::nPlayers>, THUAI4::State::nTeams>& GetPlayerGUIDs() const = 0;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const = 0;
};

class AIBase {
public:
	virtual void play(GameApi& g) = 0;
};
using CreateAIFunc = AIBase * (*)();
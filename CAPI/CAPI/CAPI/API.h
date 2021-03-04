#pragma once

#include<string>
#include<concurrent_queue.h>
#include"proto/Message2Server.pb.h"
#include<HPSocket/HPSocket.h>
#include<HPSocket/SocketInterface.h>
#include<functional>
#include"Structures.h"
#include<cstdint>
#include"Base.h"

class Logic;

class API:public GameApi
{
public:
	API(const int32_t&, const int32_t&, std::function<void(const Protobuf::MessageToServer&)>, THUAI4::State*&, std::function<void(std::string)>&);

	virtual void MovePlayer(int timeInMilliseconds, double angle);
	virtual void MoveRight(int timeInMilliseconds);
	virtual void MoveUp(int timeInMilliseconds);
	virtual void MoveLeft(int timeInMilliseconds);
	virtual void MoveDown(int timeInMilliseconds);
	virtual void Use();
	virtual void Pick(Protobuf::PropType propType);
	virtual void Throw(int timeInMilliseconds, double angle);
	virtual void Attack(int timeInMilliseconds, double angle);
	virtual void Send(int toPlayerID, std::string message);

	//选手可获取的信息
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
	virtual const std::array<std::array<uint32_t, THUAI4::State::nPlayers>, THUAI4::State::nTeams>& GetPlayerGUIDs() const;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const;
};


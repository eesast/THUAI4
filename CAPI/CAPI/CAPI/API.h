#pragma once

#ifndef API_H

#define API_H

#include <string>
#include "concurrent_queue.hpp"
#include "proto/Message2Server.pb.h"
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <functional>
#include "Structures.h"
#include <cstdint>

class Logic;

class API
{
public:

	API(const int32_t&, const int32_t&, std::function<void(const Protobuf::MessageToServer&)>, THUAI4::State*&, std::function<void(std::string)>&);
  
private:

	const int32_t& playerID;
	const int32_t& teamID;
	THUAI4::State*& pState;
	const std::function<void(const Protobuf::MessageToServer&)> SendMessage;
	concurrency::concurrent_queue<std::string> MessageStorage;
 
protected:


	void MovePlayer(int timeInMilliseconds, double angle);
	void MoveRight(int timeInMilliseconds);
	void MoveUp(int timeInMilliseconds);
	void MoveLeft(int timeInMilliseconds);
	void MoveDown(int timeInMilliseconds);
	void Use();
	void Pick(Protobuf::PropType propType);
	void Throw(int timeInMilliseconds, double angle);
	void Attack(int timeInMilliseconds, double angle);
	void Send(int toPlayerID, std::string message);


	//Ñ¡ÊÖ¿É»ñÈ¡µÄÐÅÏ¢

	bool MessageAvailable();
	bool TryGetMessage(std::string&);

	std::vector<const THUAI4::Character*> GetCharacters() const;
	std::vector<const THUAI4::Wall*> GetWalls() const;
	std::vector<const THUAI4::Prop*> GetProps() const;
	std::vector<const THUAI4::Bullet*> GetBullets() const;
	std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const;
	const THUAI4::Character& GetSelfInfo() const;
	THUAI4::ColorType GetSelfTeamColor() const;
	uint32_t GetTeamScore() const;
	const std::array<std::array<uint32_t, THUAI4::State::nPlayers>, THUAI4::State::nTeams>& GetPlayerGUIDs() const;
	const THUAI4::ColorType GetCellColor(int CellX, int CellY) const;

public:
	virtual void play() = 0;
};

#endif

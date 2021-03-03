#pragma once
#include<list>
#include<string>
#include"proto/Message2Server.pb.h"
#include<HPSocket/HPSocket.h>
#include<HPSocket/SocketInterface.h>
#include"Structures.h"
class Logic;
class API {
public:
	API(Logic* l) :plogic(l) {}
private:
	int64_t playerID = ~0;
	int64_t teamID = ~0;
	Logic* const plogic;

	//选手API 
protected:
	//选手可进行的操作
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
	//选手可获取的信息

	//据网上说vector作为返回值不会拷贝构造，那还比较合理
	std::vector<const THUAI4::Character*> getCharacters() const;
	std::vector<const THUAI4::Wall*> getWalls() const;
	std::vector<const THUAI4::Prop*> getProps() const;
	std::vector<const THUAI4::Bullet*> getBullets() const;
	std::vector<const THUAI4::BirthPoint*> getBirthPoints() const;
	const THUAI4::Character& getSelfInfo() const;
	THUAI4::ColorType getSelfTeamColor() const;
	uint32_t getTeamScore() const;
	const std::array<std::array<uint32_t, THUAI4::State::nPlayers>, THUAI4::State::nTeams>& getPlayerGUIDs() const;
	const std::array<std::array<THUAI4::ColorType, THUAI4::State::nCells>, THUAI4::State::nCells>& getCellColors() const;

public:
	virtual void play() = 0;
};


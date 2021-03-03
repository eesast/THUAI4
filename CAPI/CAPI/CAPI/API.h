#pragma once
#include<string>
#include"proto/Message2Server.pb.h"
#include<HPSocket/HPSocket.h>
#include<HPSocket/SocketInterface.h>
#include<functional>
#include"Structures.h"
#include<cstdint>
class Logic;
class API {
public:
	API(const int32_t&, const int32_t&, std::function<void(const Protobuf::MessageToServer&)>, const THUAI4::State*);
private:
	const int32_t& playerID;
	const int32_t& teamID;
	const THUAI4::State*& pState;//指针的常引用
	const std::function<void(const Protobuf::MessageToServer&)> SendMessage;// \xfgg/

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
	std::vector<const THUAI4::Character*> GetCharacters() const;
	std::vector<const THUAI4::Wall*> GetWalls() const;
	std::vector<const THUAI4::Prop*> GetProps() const;
	std::vector<const THUAI4::Bullet*> GetBullets() const;
	std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const;
	const THUAI4::Character& GetSelfInfo() const;
	THUAI4::ColorType GetSelfTeamColor() const;
	uint32_t GetTeamScore() const;
	const std::array<std::array<uint32_t, THUAI4::State::nPlayers>, THUAI4::State::nTeams>& GetPlayerGUIDs() const;
	const std::array<std::array<THUAI4::ColorType, THUAI4::State::nCells>, THUAI4::State::nCells>& GetCellColors() const;

public:
	virtual void play() = 0;
};


#pragma once
#include<cstdint>
#include<string>
#include<vector>
#include<functional>
#include"Structures.h"
#include"Constants.h"
#include"proto/Message2Server.pb.h"

struct State;

class GameApi{
protected:
	const std::function<void(Protobuf::MessageToServer&)> SendMessageWrapper;//加入ID放到这个函数里了
	const std::function<bool()> Empty;
	const std::function<bool(std::string&)> TryPop;
	const State*& pState;
public:

	GameApi(std::function<void(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp,
		const State*& pS):
		SendMessageWrapper(sm), Empty(e), TryPop(tp), pState(pS) {}
	virtual void MovePlayer(uint32_t timeInMilliseconds, double angle) = 0;
	virtual void MoveRight(uint32_t timeInMilliseconds) = 0;;
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
	virtual bool TryGetMessage(std::string&) = 0;
	virtual std::vector<const THUAI4::Character*> GetCharacters() const = 0;
	virtual std::vector<const THUAI4::Wall*> GetWalls() const = 0;
	virtual std::vector<const THUAI4::Prop*> GetProps() const = 0;
	virtual std::vector<const THUAI4::Bullet*> GetBullets() const = 0;
	virtual std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const = 0;
	virtual const THUAI4::Character& GetSelfInfo() const = 0;
	virtual THUAI4::ColorType GetSelfTeamColor() const = 0;
	virtual uint32_t GetTeamScore() const = 0;
	virtual const std::array<std::array<uint32_t, Constants::numOfPlayer>, Constants::numOfTeam>& GetPlayerGUIDs() const = 0;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const = 0;
};

//debug模式下想在Logic里调用某些方法而不把它暴露给用户
//这个实现比较愚蠢，但我没想到更好的方法……
struct LogicInterface:public GameApi {
public:
	LogicInterface(std::function<void(Protobuf::MessageToServer&)> sm,
		std::function<bool()> e, std::function<bool(std::string&)> tp,
		const State*& pS) :GameApi(sm,e,tp,pS){}
	virtual void StartTimer() = 0;
	virtual void EndTimer() = 0;
private:
	virtual void MovePlayer(uint32_t timeInMilliseconds, double angle) = 0;
	virtual void MoveRight(uint32_t timeInMilliseconds) = 0;;
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
	virtual bool TryGetMessage(std::string&) = 0;
	virtual std::vector<const THUAI4::Character*> GetCharacters() const = 0;
	virtual std::vector<const THUAI4::Wall*> GetWalls() const = 0;
	virtual std::vector<const THUAI4::Prop*> GetProps() const = 0;
	virtual std::vector<const THUAI4::Bullet*> GetBullets() const = 0;
	virtual std::vector<const THUAI4::BirthPoint*> GetBirthPoints() const = 0;
	virtual const THUAI4::Character& GetSelfInfo() const = 0;
	virtual THUAI4::ColorType GetSelfTeamColor() const = 0;
	virtual uint32_t GetTeamScore() const = 0;
	virtual const std::array<std::array<uint32_t, Constants::numOfPlayer>, Constants::numOfTeam>& GetPlayerGUIDs() const = 0;
	virtual THUAI4::ColorType GetCellColor(int CellX, int CellY) const = 0;

};


class AIBase {
public:
	virtual void play(GameApi& g) = 0;
};
using CreateAIFunc = AIBase * (*)();

int thuai4_main(int argc, char** argv, CreateAIFunc AIBuilder);
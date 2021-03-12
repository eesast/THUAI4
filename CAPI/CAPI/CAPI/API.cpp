#include"API.h"
#include "Constants.h"
#include <functional>
#include <chrono>
#include <ctime>


const static double PI = 3.14159265358979323846;

double TimeSinceStart(const std::chrono::system_clock::time_point& sp)
{
	std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	std::chrono::duration<double,std::milli> time_span = std::chrono::duration_cast<std::chrono::duration<double,std::milli>>(tp - sp);
	return time_span.count();
}

API::API(std::function<void(Protobuf::MessageToServer&)> sm,
	std::function<bool()> e, std::function<bool(std::string&)> tp,
	const State*& pS) :LogicInterface(sm, e, tp, pS) {}

void API::Use()
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Use);
	SendMessageWrapper(message);
}
void API::Pick(THUAI4::PropType propType)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Pick);
	message.set_proptype(Protobuf::PropType(propType));
	SendMessageWrapper(message);
}
void API::Throw(uint32_t timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Throw);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	SendMessageWrapper(message);
}
void API::Attack(uint32_t timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Attack);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	SendMessageWrapper(message);
}
void API::Send(int toPlayerID, std::string message)
{
	Protobuf::MessageToServer msg;
	msg.set_messagetype(Protobuf::MessageType::Send);
	msg.set_toplayerid(toPlayerID);
	msg.set_message(message);
	SendMessageWrapper(msg);
}
void API::MovePlayer(uint32_t timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Move);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	SendMessageWrapper(message);
}
void API::MoveRight(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI * 0.5);
}
void API::MoveUp(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI);
}
void API::MoveLeft(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI * 1.5);
}
void API::MoveDown(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, 0);
}
bool API::MessageAvailable()
{
	return !Empty();
}
bool API::TryGetMessage(std::string& str)
{
	return TryPop(str);
}

std::vector<const THUAI4::Character*> API::GetCharacters() const
{
	std::vector<const THUAI4::Character*> characters;

	for (auto it : pState->characters) {
		characters.push_back(it.get());
	}
	return characters;

}
std::vector<const THUAI4::Wall*> API::GetWalls() const
{
	std::vector<const THUAI4::Wall*> walls;
	for (auto it : pState->walls) {
		walls.push_back(it.get());
	}
	return walls;
}
std::vector<const THUAI4::Prop*> API::GetProps() const
{
	std::vector<const THUAI4::Prop*> props;

	for (auto it : pState->props) {
		props.push_back(it.get());
	}
	return props;
}
std::vector<const THUAI4::Bullet*> API::GetBullets() const
{
	std::vector<const THUAI4::Bullet*> bullets;

	for (auto it : pState->bullets) {
		bullets.push_back(it.get());
	}
	return bullets;
}
std::vector<const THUAI4::BirthPoint*> API::GetBirthPoints() const
{
	std::vector<const THUAI4::BirthPoint*> birthpoints;
	for (auto it : pState->birthpoints) {
		birthpoints.push_back(it.get());
	}
	return birthpoints;
}
const THUAI4::Character& API::GetSelfInfo() const
{
	return *pState->self;
}
THUAI4::ColorType API::GetSelfTeamColor() const
{
	return pState->selfTeamColor;
}
uint32_t API::GetTeamScore() const
{
	return pState->teamScore;
}
const std::array<std::array<uint32_t, StateConstant::nPlayers>, StateConstant::nTeams>& API::GetPlayerGUIDs() const
{
	return pState->playerGUIDs;
}
THUAI4::ColorType API::GetCellColor(int CellX, int CellY) const
{
	assert(CellX >= 0 && CellX < StateConstant::nCells&& CellY >= 0 && CellY < StateConstant::nCells);
#ifdef _COLOR_MAP_BY_HASHING_
	auto it = pState->cellColors.find((CellX << 16) + CellY);
	if (it == pState->cellColors.end()) {
		return THUAI4::ColorType::Invisible;
	}
	return it->second;
#else
	return pState->cellColors[CellX][CellY];
#endif // _COLOR_MAP_BY_HASHING_

}

//Debug API
//目前实现的功能：调用函数都留下记录、可选合法性检查、记录每次play用时

DebugApi::DebugApi(std::function<void(Protobuf::MessageToServer&)> sm,
	std::function<bool()> e, std::function<bool(std::string&)> tp,
	const State*& pS,
	bool ev
	, std::ostream& out) :
	LogicInterface(sm, e, tp, pS), ExamineValidity(ev), OutStream(out) {}

void DebugApi::StartTimer()
{
	StartPoint = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(StartPoint);
	OutStream << "===New State===" << std::endl;
	OutStream << "Current time: " << ctime(&t);
}

void DebugApi::EndTimer()
{
	OutStream << "Time elapsed: " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	OutStream << std::endl;
}

void DebugApi::Use()
{
	OutStream << "Call Use() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	if (ExamineValidity) {
		if (pState->self->isDying) {
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return;//不合法发了也是白发
		}
		if (pState->self->propType == THUAI4::PropType::Null) {
			OutStream << "[Warning: You don`t have any properties.]" << std::endl;
			return;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Use);
	SendMessageWrapper(message);
}
inline bool InSameCell(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
	return (x1 / Constants::Map::numOfGridPerCell == x2 / Constants::Map::numOfGridPerCell) ||
		(y1 / Constants::Map::numOfGridPerCell == y2 / Constants::Map::numOfGridPerCell);
}
bool DebugApi::CanPick(THUAI4::PropType propType)
{
	for (auto it : pState->props) {
		if (InSameCell(pState->self->x, pState->self->y, it->x, it->y) && propType == it->propType) {
			return true;
		}
	}
	return false;
}
void DebugApi::Pick(THUAI4::PropType propType)
{
	OutStream << "Call Pick(" << dict[propType] << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	if (ExamineValidity) {
		if (pState->self->isDying) {
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return;
		}
		if (CanPick(propType)) {
			OutStream << "[Warning: No such property to pick within the cell.]" << std::endl;
			return;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Pick);
	message.set_proptype(Protobuf::PropType(propType));
	SendMessageWrapper(message);
}
void DebugApi::Throw(uint32_t timeInMilliseconds, double angle)
{
	OutStream << "Call Throw(" << timeInMilliseconds << "," << angle << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	if (ExamineValidity) {
		if (pState->self->isDying) {
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return;
		}
		if (pState->self->propType == THUAI4::PropType::Null) {
			OutStream << "[Warning: Nothing to throw.]" << std::endl;
			return;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Throw);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	SendMessageWrapper(message);
}
void DebugApi::Attack(uint32_t timeInMilliseconds, double angle)
{
	OutStream << "Call Attack(" << timeInMilliseconds << "," << angle << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	if (ExamineValidity) {
		if (pState->self->isDying) {
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return;
		}
		if (pState->self->bulletNum == 0) {
			OutStream << "[Warning: You are out of bullets.]" << std::endl;
			return;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Attack);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	SendMessageWrapper(message);
}
void DebugApi::Send(int toPlayerID, std::string message)
{
	OutStream << "Call Send(" << toPlayerID << "," << message << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	if (ExamineValidity) {//应该没啥必要
		if (toPlayerID < 0 || toPlayerID >= StateConstant::nPlayers) {
			OutStream << "[Warning: Illegal player ID.]" << std::endl;
			return;
		}
		if (pState->playerGUIDs[pState->self->teamID][toPlayerID] == pState->self->guid) {
			OutStream << "[Warning: You are sending a message to yourself.]" << std::endl;
			return;
		}
	}
	Protobuf::MessageToServer msg;
	msg.set_messagetype(Protobuf::MessageType::Send);
	msg.set_toplayerid(toPlayerID);
	msg.set_message(message);
	SendMessageWrapper(msg);
}
void DebugApi::MovePlayer(uint32_t timeInMilliseconds, double angle)
{
	OutStream << "Call MovePlayer(" << timeInMilliseconds << "," << angle << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	if (ExamineValidity) {
		if (pState->self->isDying) {
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Move);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	SendMessageWrapper(message);
}
void DebugApi::MoveRight(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI * 0.5);
}
void DebugApi::MoveUp(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI);
}
void DebugApi::MoveLeft(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI * 1.5);
}
void DebugApi::MoveDown(uint32_t timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, 0);
}
bool DebugApi::MessageAvailable()
{
	return !Empty();
}
bool DebugApi::TryGetMessage(std::string& str)
{
	OutStream << "Call TryGetMessage() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	bool res = TryPop(str);
	if (ExamineValidity) {
		if (!res) {
			OutStream << "[Warning: Failed to get a message.]" << std::endl;
		}
	}
	return res;
}

std::vector<const THUAI4::Character*> DebugApi::GetCharacters() const
{
	OutStream << "Call GetCharacters() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	std::vector<const THUAI4::Character*> characters;

	for (auto it : pState->characters) {
		characters.push_back(it.get());
	}
	return characters;

}
std::vector<const THUAI4::Wall*> DebugApi::GetWalls() const
{
	OutStream << "Call GetWalls() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	std::vector<const THUAI4::Wall*> walls;
	for (auto it : pState->walls) {
		walls.push_back(it.get());
	}
	return walls;
}
std::vector<const THUAI4::Prop*> DebugApi::GetProps() const
{
	OutStream << "Call GetProps() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	std::vector<const THUAI4::Prop*> props;
	for (auto it : pState->props) {
		props.push_back(it.get());
	}
	return props;
}
std::vector<const THUAI4::Bullet*> DebugApi::GetBullets() const
{
	OutStream << "Call GetBullets() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	std::vector<const THUAI4::Bullet*> bullets;
	for (auto it : pState->bullets) {
		bullets.push_back(it.get());
	}
	return bullets;
}
std::vector<const THUAI4::BirthPoint*> DebugApi::GetBirthPoints() const
{
	OutStream << "Call GetBirthPoints() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	std::vector<const THUAI4::BirthPoint*> birthpoints;
	for (auto it : pState->birthpoints) {
		birthpoints.push_back(it.get());
	}
	return birthpoints;
}
const THUAI4::Character& DebugApi::GetSelfInfo() const
{
	OutStream << "Call GetSelfInfo() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	return *pState->self;
}
THUAI4::ColorType DebugApi::GetSelfTeamColor() const
{
	OutStream << "Call GetSelfTeamColor() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	return pState->selfTeamColor;
}
uint32_t DebugApi::GetTeamScore() const
{
	OutStream << "Call GetTeamScore() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	return pState->teamScore;
}
const std::array<std::array<uint32_t, StateConstant::nPlayers>, StateConstant::nTeams>& DebugApi::GetPlayerGUIDs() const
{
	OutStream << "Call GetPlayerGUIDs() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	return pState->playerGUIDs;
}

THUAI4::ColorType DebugApi::GetCellColor(int CellX, int CellY) const
{
	OutStream << "Call GetCellColor(" << CellX << "," << CellY << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;;
	assert(CellX >= 0 && CellX < StateConstant::nCells&& CellY >= 0 && CellY < StateConstant::nCells);
	//非法直接就炸了，不用检查
	if (ExamineValidity) {
		if (!CellColorVisible(pState->self->x, pState->self->y, CellX, CellY)) {
			OutStream << "[Warning: This cell is invisible.]" << std::endl;
		}
	}

#ifdef _COLOR_MAP_BY_HASHING_
	auto it = pState->cellColors.find((CellX << 16) + CellY);
	if (it == pState->cellColors.end()) {
		return THUAI4::ColorType::Invisible;
	}
	return it->second;
#else
	return pState->cellColors[CellX][CellY];
#endif // _COLOR_MAP_BY_HASHING_

}

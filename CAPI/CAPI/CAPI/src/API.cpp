#include "API.h"
#include "Constants.h"
#include <functional>
#include <chrono>
#include <ctime>
#include <utility>

std::vector<std::vector<int64_t>> State::playerGUIDs;
const static double PI = 3.14159265358979323846;

double TimeSinceStart(const std::chrono::system_clock::time_point& sp)
{
	std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> time_span = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(tp - sp);
	return time_span.count();
}



template <bool asyn>
API<asyn>::API(std::function<bool(Protobuf::MessageToServer&)> sm,
	std::function<bool()> e, std::function<bool(std::string&)> tp, std::function<int()> gc,
	const State*& pS, std::mutex& mtx_state, std::function<void()> tu, std::function<void()> w) : LogicInterface(sm, e, tp, gc, pS, w), Members<asyn>(mtx_state, tu)
{

}

template <bool asyn>
bool API<asyn>::Use()
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Use);
	return SendMessageWrapper(message);
}
template <bool asyn>
bool API<asyn>::Pick(THUAI4::PropType propType)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Pick);
	message.set_proptype(Protobuf::PropType(propType));
	return SendMessageWrapper(message);
}
template <bool asyn>
bool API<asyn>::Throw(uint32_t timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Throw);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	return SendMessageWrapper(message);
}
template <bool asyn>
bool API<asyn>::Attack(uint32_t timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Attack);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	return SendMessageWrapper(message);
}
template <bool asyn>
bool API<asyn>::Send(int toPlayerID, std::string message)
{
	Protobuf::MessageToServer msg;
	msg.set_messagetype(Protobuf::MessageType::Send);
	msg.set_toplayerid(toPlayerID);
	msg.set_message(message);
	return SendMessageWrapper(msg);
}
template <bool asyn>
bool API<asyn>::MovePlayer(uint32_t timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Move);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	return SendMessageWrapper(message);
}
template <bool asyn>
bool API<asyn>::MoveRight(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, PI * 0.5);
}
template <bool asyn>
bool API<asyn>::MoveUp(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, PI);
}
template <bool asyn>
bool API<asyn>::MoveLeft(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, PI * 1.5);
}
template <bool asyn>
bool API<asyn>::MoveDown(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, 0);
}
template <bool asyn>
bool API<asyn>::Wait()
{
	if (GetCounter() == -1) return false;
	if constexpr (asyn) {
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		_Wait();
		return true;
	}
	else {
		_Wait();
		return true;
	}
}


template <bool asyn>
int API<asyn>::GetCounterOfFrames()
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		return GetCounter();
	}
	else
		return GetCounter();
}

template <bool asyn>
bool API<asyn>::MessageAvailable()
{
	return !Empty();
}
template <bool asyn>
bool API<asyn>::TryGetMessage(std::string& str)
{
	return TryPop(str);
}

template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Character>> API<asyn>::GetCharacters() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		std::vector<std::shared_ptr<const THUAI4::Character>> temp;
		temp.assign(pState->characters.begin(), pState->characters.end());
		return temp;
	}
	else {
		std::vector<std::shared_ptr<const THUAI4::Character>> temp;
		temp.assign(pState->characters.begin(), pState->characters.end());
		return temp;
	}
}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Wall>> API<asyn>::GetWalls() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		std::vector<std::shared_ptr<const THUAI4::Wall>> temp;
		temp.assign(pState->walls.begin(), pState->walls.end());
		return temp;
	}
	else {
		std::vector<std::shared_ptr<const THUAI4::Wall>> temp;
		temp.assign(pState->walls.begin(), pState->walls.end());
		return temp;
	}
}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Prop>> API<asyn>::GetProps() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		std::vector<std::shared_ptr<const THUAI4::Prop>> temp;
		temp.assign(pState->props.begin(), pState->props.end());
		return temp;
	}
	else {
		std::vector<std::shared_ptr<const THUAI4::Prop>> temp;
		temp.assign(pState->props.begin(), pState->props.end());
		return temp;
	}

}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Bullet>> API<asyn>::GetBullets() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		std::vector<std::shared_ptr<const THUAI4::Bullet>> temp;
		temp.assign(pState->bullets.begin(), pState->bullets.end());
		return temp;
	}
	else {
		std::vector<std::shared_ptr<const THUAI4::Bullet>> temp;
		temp.assign(pState->bullets.begin(), pState->bullets.end());
		return temp;
	}

}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::BirthPoint>> API<asyn>::GetBirthPoints() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		std::vector<std::shared_ptr<const THUAI4::BirthPoint>> temp;
		temp.assign(pState->birthpoints.begin(), pState->birthpoints.end());
		return temp;
	}
	else {
		std::vector<std::shared_ptr<const THUAI4::BirthPoint>> temp;
		temp.assign(pState->birthpoints.begin(), pState->birthpoints.end());
		return temp;
	}

}
template <bool asyn>
std::shared_ptr<const THUAI4::Character> API<asyn>::GetSelfInfo() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		return pState->self;
	}
	else
		return pState->self;
}

template <bool asyn>
THUAI4::ColorType API<asyn>::GetSelfTeamColor() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		return pState->selfTeamColor;
	}
	else
		return pState->selfTeamColor;
}
template <bool asyn>
uint32_t API<asyn>::GetTeamScore() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		return pState->teamScore;
	}
	else
		return pState->teamScore;
}
template <bool asyn>
const std::vector<std::vector<int64_t>> API<asyn>::GetPlayerGUIDs() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		return State::playerGUIDs;
	}
	else
		return State::playerGUIDs;
}
template <bool asyn>
THUAI4::ColorType API<asyn>::GetCellColor(int CellX, int CellY) const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
#ifdef _COLOR_MAP_BY_HASHING_
		auto it = pState->cellColors.find((CellX << 16) + CellY);
		if (it == pState->cellColors.end())
		{
			return THUAI4::ColorType::Invisible;
		}
		return it->second;
#else
		return pState->cellColors[CellX][CellY];
#endif // _COLOR_MAP_BY_HASHING_
		}
	else {
#ifdef _COLOR_MAP_BY_HASHING_
		auto it = pState->cellColors.find((CellX << 16) + CellY);
		if (it == pState->cellColors.end())
		{
			return THUAI4::ColorType::Invisible;
		}
		return it->second;
#else
		return pState->cellColors[CellX][CellY];
#endif // _COLOR_MAP_BY_HASHING_
		}

	}


//Debug API
//目前实现的功能：调用函数都留下记录、可选合法性检查、记录每次play用时
template <bool asyn>
DebugApi<asyn>::DebugApi(std::function<bool(Protobuf::MessageToServer&)> sm,
	std::function<bool()> e, std::function<bool(std::string&)> tp, std::function<int()> gc,
	const State*& pS, std::mutex& mtx_state, std::function<void()> tu, std::function<void()> w, bool ev,
	std::ostream& out) : LogicInterface(sm, e, tp, gc, pS, w), Members<asyn>(mtx_state, tu),
	ExamineValidity(ev), OutStream(out)
{

}

template <bool asyn>
void DebugApi<asyn>::StartTimer()
{
	StartPoint = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(StartPoint);
	OutStream << "=== AI.play() ===" << std::endl;
	OutStream << "Current time: " << ctime(&t);
}
template <bool asyn>
void DebugApi<asyn>::EndTimer()
{
	OutStream << "Time elapsed: " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	OutStream << std::endl;
}
template <bool asyn>
bool DebugApi<asyn>::Use()
{
	OutStream << "Call Use() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	if (ExamineValidity)
	{
		if (pState->self->isDying)
		{
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return false; //不合法发了也是白发
		}
		if (pState->self->propType == THUAI4::PropType::Null)
		{
			OutStream << "[Warning: You don`t have any properties.]" << std::endl;
			return false;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Use);
	return SendMessageWrapper(message);
}
inline bool InSameCell(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
	return (x1 / Constants::Map::numOfGridPerCell == x2 / Constants::Map::numOfGridPerCell) ||
		(y1 / Constants::Map::numOfGridPerCell == y2 / Constants::Map::numOfGridPerCell);
}
template <bool asyn>
bool DebugApi<asyn>::CanPick(THUAI4::PropType propType)
{
	for (auto it : pState->props)
	{
		if (InSameCell(pState->self->x, pState->self->y, it->x, it->y) && propType == it->propType)
		{
			return true;
		}
	}
	return false;
}
template <bool asyn>
bool DebugApi<asyn>::Pick(THUAI4::PropType propType)
{
	OutStream << "Call Pick(" << dict[propType] << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	if (ExamineValidity)
	{
		if (pState->self->isDying)
		{
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return false;
		}
		if (!CanPick(propType))
		{
			OutStream << "[Warning: No such property to pick within the cell.]" << std::endl;
			return false;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Pick);
	message.set_proptype(Protobuf::PropType(propType));
	return SendMessageWrapper(message);
}
template <bool asyn>
bool DebugApi<asyn>::Throw(uint32_t timeInMilliseconds, double angle)
{
	OutStream << "Call Throw(" << timeInMilliseconds << "," << angle << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	if (ExamineValidity)
	{
		if (pState->self->isDying)
		{
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return false;
		}
		if (pState->self->propType == THUAI4::PropType::Null)
		{
			OutStream << "[Warning: Nothing to throw.]" << std::endl;
			return false;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Throw);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	return SendMessageWrapper(message);
}
template <bool asyn>
bool DebugApi<asyn>::Attack(uint32_t timeInMilliseconds, double angle)
{
	OutStream << "Call Attack(" << timeInMilliseconds << "," << angle << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	if (ExamineValidity)
	{
		if (pState->self->isDying)
		{
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return false;
		}
		if (pState->self->bulletNum == 0)
		{
			OutStream << "[Warning: You are out of bullets.]" << std::endl;
			return false;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Attack);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	return SendMessageWrapper(message);
}
template <bool asyn>
bool DebugApi<asyn>::Send(int toPlayerID, std::string message)
{
	OutStream << "Call Send(" << toPlayerID << "," << message << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	if (ExamineValidity)
	{ //应该没啥必要
		if (toPlayerID < 0 || toPlayerID >= StateConstant::nPlayers)
		{
			OutStream << "[Error: Illegal player ID.]" << std::endl;
			return false;
		}
		if (State::playerGUIDs[pState->self->teamID][toPlayerID] == pState->self->guid)
		{
			OutStream << "[Warning: You are sending a message to yourself.]" << std::endl;
			// return false;
		}
	}
	Protobuf::MessageToServer msg;
	msg.set_messagetype(Protobuf::MessageType::Send);
	msg.set_toplayerid(toPlayerID);
	msg.set_message(message);
	return SendMessageWrapper(msg);
}
template <bool asyn>
bool DebugApi<asyn>::MovePlayer(uint32_t timeInMilliseconds, double angle)
{
	OutStream << "Call MovePlayer(" << timeInMilliseconds << "," << angle << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;

	if (ExamineValidity)
	{
		if (pState->self->isDying)
		{
			OutStream << "[Warning: You have been slained.]" << std::endl;
			return false;
		}
	}
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Move);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	return SendMessageWrapper(message);
}
template <bool asyn>
bool DebugApi<asyn>::MoveRight(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, PI * 0.5);
}
template <bool asyn>
bool DebugApi<asyn>::MoveUp(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, PI);
}
template <bool asyn>
bool DebugApi<asyn>::MoveLeft(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, PI * 1.5);
}
template <bool asyn>
bool DebugApi<asyn>::MoveDown(uint32_t timeInMilliseconds)
{
	return MovePlayer(timeInMilliseconds, 0);
}
template <bool asyn>
bool DebugApi<asyn>::Wait()
{
	OutStream << "Call Wait() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	if (GetCounter() == -1) return false;
	if constexpr (asyn) {
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		_Wait();
		return true;
	}
	else {
		_Wait();
		return true;
	}
}

template <bool asyn>
int DebugApi<asyn>::GetCounterOfFrames()
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetCounterOfFrames() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return GetCounter();
	}
	else {
		OutStream << "Call GetCounterOfFrames() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return GetCounter();
	}

}
template <bool asyn>
bool DebugApi<asyn>::MessageAvailable()
{
	OutStream << "Call MessageAvailable() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	return !Empty();
}
template <bool asyn>
bool DebugApi<asyn>::TryGetMessage(std::string& str)
{
	OutStream << "Call TryGetMessage() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
	bool res = TryPop(str);
	if (ExamineValidity)
	{
		if (!res)
		{
			OutStream << "[Warning: Failed to get a message.]" << std::endl;
		}
	}
	return res;
}

template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Character>> DebugApi<asyn>::GetCharacters() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetCharacters() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Character>> temp;
		temp.assign(pState->characters.begin(), pState->characters.end());
		return temp;
	}
	else {
		OutStream << "Call GetCharacters() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Character>> temp;
		temp.assign(pState->characters.begin(), pState->characters.end());
		return temp;
	}

}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Wall>> DebugApi<asyn>::GetWalls() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetWalls() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Wall>> temp;
		temp.assign(pState->walls.begin(), pState->walls.end());
		return temp;
	}
	else {
		OutStream << "Call GetWalls() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Wall>> temp;
		temp.assign(pState->walls.begin(), pState->walls.end());
		return temp;
	}

}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Prop>> DebugApi<asyn>::GetProps() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetProps() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Prop>> temp;
		temp.assign(pState->props.begin(), pState->props.end());
		return temp;
	}
	else {
		OutStream << "Call GetProps() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Prop>> temp;
		temp.assign(pState->props.begin(), pState->props.end());
		return temp;
	}

}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::Bullet>> DebugApi<asyn>::GetBullets() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetBullets() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Bullet>> temp;
		temp.assign(pState->bullets.begin(), pState->bullets.end());
		return temp;
	}
	else {
		OutStream << "Call GetBullets() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::Bullet>> temp;
		temp.assign(pState->bullets.begin(), pState->bullets.end());
		return temp;
	}

}
template <bool asyn>
std::vector<std::shared_ptr<const THUAI4::BirthPoint>> DebugApi<asyn>::GetBirthPoints() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetBirthPoints() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::BirthPoint>> temp;
		temp.assign(pState->birthpoints.begin(), pState->birthpoints.end());
		return temp;
	}
	else {
		OutStream << "Call GetBirthPoints() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		std::vector<std::shared_ptr<const THUAI4::BirthPoint>> temp;
		temp.assign(pState->birthpoints.begin(), pState->birthpoints.end());
		return temp;
	}

}
template <bool asyn>
std::shared_ptr<const THUAI4::Character> DebugApi<asyn>::GetSelfInfo() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetSelfInfo() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return pState->self;
	}
	else {
		OutStream << "Call GetSelfInfo() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return pState->self;
	}

}

template <bool asyn>
THUAI4::ColorType DebugApi<asyn>::GetSelfTeamColor() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetSelfTeamColor() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return pState->selfTeamColor;
	}
	else {
		OutStream << "Call GetSelfTeamColor() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return pState->selfTeamColor;
	}

}
template <bool asyn>
uint32_t DebugApi<asyn>::GetTeamScore() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetTeamScore() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return pState->teamScore;
	}
	else {
		OutStream << "Call GetTeamScore() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return pState->teamScore;
	}

}
template <bool asyn>
const std::vector<std::vector<int64_t>> DebugApi<asyn>::GetPlayerGUIDs() const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetPlayerGUIDs() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return State::playerGUIDs;
	}
	else {
		OutStream << "Call GetPlayerGUIDs() at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		return State::playerGUIDs;
	}

}
template <bool asyn>
THUAI4::ColorType DebugApi<asyn>::GetCellColor(int CellX, int CellY) const
{
	if constexpr (asyn)
	{
		std::lock_guard<std::mutex> lck(Members<asyn>::mtx_state);
		Members<asyn>::TryUpDate();
		OutStream << "Call GetCellColor(" << CellX << "," << CellY << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		assert(CellX >= 0 && CellX < StateConstant::nCells&& CellY >= 0 && CellY < StateConstant::nCells);
		//非法直接就炸了，不用检查
		if (ExamineValidity)
		{
			if (!CellColorVisible(pState->self->x, pState->self->y, CellX, CellY))
			{
				OutStream << "[Warning: This cell is invisible.]" << std::endl;
			}
		}

#ifdef _COLOR_MAP_BY_HASHING_
		auto it = pState->cellColors.find((CellX << 16) + CellY);
		if (it == pState->cellColors.end())
		{
			return THUAI4::ColorType::Invisible;
		}
		return it->second;
#else
		return pState->cellColors[CellX][CellY];
#endif // _COLOR_MAP_BY_HASHING_
		}
	else {
		OutStream << "Call GetCellColor(" << CellX << "," << CellY << ") at " << TimeSinceStart(StartPoint) << "ms" << std::endl;
		assert(CellX >= 0 && CellX < StateConstant::nCells&& CellY >= 0 && CellY < StateConstant::nCells);
		//非法直接就炸了，不用检查
		if (ExamineValidity)
		{
			if (!CellColorVisible(pState->self->x, pState->self->y, CellX, CellY))
			{
				OutStream << "[Warning: This cell is invisible.]" << std::endl;
			}
		}

#ifdef _COLOR_MAP_BY_HASHING_
		auto it = pState->cellColors.find((CellX << 16) + CellY);
		if (it == pState->cellColors.end())
		{
			return THUAI4::ColorType::Invisible;
		}
		return it->second;
#else
		return pState->cellColors[CellX][CellY];
#endif // _COLOR_MAP_BY_HASHING_
		}

	}

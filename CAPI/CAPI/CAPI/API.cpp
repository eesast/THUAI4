#include "API.h"
#include "Logic.h"

const static double PI = 3.14159265358979323846;

void API::Use()
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Use);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	logic.capi.Send(message);
}

void API::Pick(Protobuf::PropType propType)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Pick);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	message.set_proptype(propType);
	logic.capi.Send(message);
}

void API::Throw(int timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Throw);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	logic.capi.Send(message);
}

void API::Attack(int timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Attack);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	logic.capi.Send(message);
}

void API::Send(int toPlayerID, std::string message)
{
	Protobuf::MessageToServer msg;
	msg.set_messagetype(Protobuf::MessageType::Send);
	msg.set_playerid(playerID);
	msg.set_teamid(teamID);
	msg.set_toplayerid(toPlayerID);
	msg.set_message(message);
	logic.capi.Send(msg);
}

void API::MovePlayer(int timeInMilliseconds, double angle)
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::Move);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	message.set_timeinmilliseconds(timeInMilliseconds);
	message.set_angle(angle);
	logic.capi.Send(message);
}

void API::MoveRight(int timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI * 0.5);
}

void API::MoveUp(int timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI);
}

void API::MoveLeft(int timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, PI * 1.5);
}

void API::MoveDown(int timeInMilliseconds)
{
	MovePlayer(timeInMilliseconds, 0);
}

std::vector<const THUAI4::Character*> API::GetCharacters() const
{
	std::vector<const THUAI4::Character*> characters;
	for (auto it : logic.pState->characters) {
		characters.push_back(it.get());
	}
	return characters;
	
}
std::vector<const THUAI4::Wall*> API::GetWalls() const
{
	std::vector<const THUAI4::Wall*> walls;
	for (auto it : logic.pState->walls) {
		walls.push_back(it.get());
	}
	return walls;
}
std::vector<const THUAI4::Prop*> API::GetProps() const
{
	std::vector<const THUAI4::Prop*> props;
	for (auto it : logic.pState->props) {
		props.push_back(it.get());
	}
	return props;
}
std::vector<const THUAI4::Bullet*> API::GetBullets() const
{
	std::vector<const THUAI4::Bullet*> bullets;
	for (auto it : logic.pState->bullets) {
		bullets.push_back(it.get());
	}
	return bullets;
}
std::vector<const THUAI4::BirthPoint*> API::GetBirthPoints() const
{
	std::vector<const THUAI4::BirthPoint*> birthpoints;
	for (auto it : logic.pState->birthpoints) {
		birthpoints.push_back(it.get());
	}
	return birthpoints;
}
const THUAI4::Character& API::GetSelfInfo() const
{
	return *logic.pState->self;
}
THUAI4::ColorType API::GetSelfTeamColor() const
{
	return logic.pState->selfTeamColor;
}
uint32_t API::GetTeamScore() const
{
	return logic.pState->teamScore;
}
const std::array<std::array<uint32_t, THUAI4::State::nPlayers>, THUAI4::State::nTeams>& API::GetPlayerGUIDs() const
{
	return logic.pState->playerGUIDs;
}
const std::array<std::array<THUAI4::ColorType, THUAI4::State::nCells>, THUAI4::State::nCells>& API::GetCellColors() const
{
	return logic.pState->cellColors;
}

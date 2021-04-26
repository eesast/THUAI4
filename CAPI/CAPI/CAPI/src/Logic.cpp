#include "Logic.h"
#include <fstream>
//#define _ALL_VISIBLE_

extern const bool asynchronous;

//辅助函数
std::shared_ptr<THUAI4::Character> obj2C(const Protobuf::GameObjInfo& goi)
{
	std::shared_ptr<THUAI4::Character> character = std::make_shared<THUAI4::Character>();
	character->ap = goi.ap();
	character->bulletNum = goi.bulletnum();
	character->bulletType = (THUAI4::BulletType)goi.bullettype();
	character->CD = goi.cd();
	character->facingDirection = goi.facingdirection();
	character->guid = goi.guid();
	character->hp = goi.hp();
	character->isDying = goi.isdying();
	character->isMoving = goi.ismoving();
	character->jobType = (THUAI4::JobType)goi.jobtype();
	character->lifeNum = goi.lifenum();
	character->maxBulletNum = goi.maxbulletnum();
	character->maxHp = goi.maxhp();
	character->moveSpeed = goi.movespeed();
	character->propType = (THUAI4::PropType)goi.proptype();
	character->radius = goi.radius();
	character->shapeType = (THUAI4::ShapeType)goi.shapetype();
	character->teamID = static_cast<uint16_t>(goi.teamid());
	character->x = goi.x();
	character->y = goi.y();
	return character;
}
std::shared_ptr<THUAI4::Wall> obj2W(const Protobuf::GameObjInfo& goi)
{
	std::shared_ptr<THUAI4::Wall> wall = std::make_shared<THUAI4::Wall>();
	wall->guid = goi.guid();
	wall->radius = goi.radius();
	wall->shapeType = (THUAI4::ShapeType)goi.shapetype();
	wall->x = goi.x();
	wall->y = goi.y();
	return wall;
}
std::shared_ptr<THUAI4::Prop> obj2P(const Protobuf::GameObjInfo& goi)
{
	std::shared_ptr<THUAI4::Prop> prop = std::make_shared<THUAI4::Prop>();
	prop->facingDirection = goi.facingdirection();
	prop->guid = goi.guid();
	prop->isLaid = goi.islaid();
	prop->isMoving = goi.ismoving();
	prop->moveSpeed = goi.movespeed();
	prop->propType = (THUAI4::PropType)goi.proptype();
	prop->radius = goi.radius();
	prop->shapeType = (THUAI4::ShapeType)goi.shapetype();
	prop->x = goi.x();
	prop->y = goi.y();
	return prop;
}
std::shared_ptr<THUAI4::Bullet> obj2Blt(const Protobuf::GameObjInfo& goi)
{
	std::shared_ptr<THUAI4::Bullet> bullet = std::make_shared<THUAI4::Bullet>();
	bullet->ap = goi.ap();
	bullet->bulletType = (THUAI4::BulletType)goi.bullettype();
	bullet->facingDirection = goi.facingdirection();
	bullet->guid = goi.guid();
	bullet->isMoving = goi.ismoving();
	bullet->moveSpeed = goi.movespeed();
	bullet->radius = goi.radius();
	bullet->shapeType = (THUAI4::ShapeType)goi.shapetype();
	bullet->teamID = static_cast<uint16_t>(goi.teamid());
	bullet->x = goi.x();
	bullet->y = goi.y();
	return bullet;
}
std::shared_ptr<THUAI4::BirthPoint> obj2Bp(const Protobuf::GameObjInfo& goi)
{
	std::shared_ptr<THUAI4::BirthPoint> birthpoint = std::make_shared<THUAI4::BirthPoint>();
	birthpoint->guid = goi.guid();
	birthpoint->radius = goi.radius();
	birthpoint->shapeType = (THUAI4::ShapeType)goi.shapetype();
	birthpoint->teamID = static_cast<uint16_t>(goi.teamid());
	birthpoint->x = goi.x();
	birthpoint->y = goi.y();
	return birthpoint;
}
bool visible(int32_t x, int32_t y, Protobuf::GameObjInfo& g)
{
	Protobuf::GameObjType gT = g.gameobjtype();
	Protobuf::PropType pT = g.proptype();
	if (gT == Protobuf::GameObjType::Prop && g.islaid() && (pT == Protobuf::PropType::Attenuator || pT == Protobuf::PropType::Dirt || pT == Protobuf::PropType::Divider))
		return false;

	int64_t dx = x - g.x();
	int64_t dy = y - g.y();
	uint64_t distanceSquared = dx * dx + dy * dy;
	return distanceSquared <= Constants::Map::sightRadiusSquared;
}

void Logic::ProcessM2C(std::shared_ptr<Protobuf::MessageToClient> pM2C)
{
	if (pM2C->playerid() != ID::GetPlayerID() || pM2C->teamid() != ID::GetTeamID())
		return; //权宜之计
	switch (pM2C->messagetype())
	{
	case Protobuf::MessageType::StartGame:
	{
		load(pM2C);

		//playerGuid只在这里记录
		State::playerGUIDs.clear();
		for (auto i : pM2C->playerguids()) {
			State::playerGUIDs.push_back(std::vector<int64_t>(i.teammateguids().cbegin(), i.teammateguids().cend()));
		}
		sw_AI = true;
		UnBlockAI();
		std::cout << "游戏开始" << std::endl;
		break;
	}
	case Protobuf::MessageType::Gaming:
		load(pM2C);
		break;

	case Protobuf::MessageType::EndGame:
	{
		sw_AI = false;
		std::cout << "游戏结束" << std::endl;
		{
			std::lock_guard<std::mutex> lck(mtx_buffer);
			FlagBufferUpdated = true;
			counter_buffer = -1;
		}
		cv_buffer.notify_one();
		break;
	}

	default:
		std::cout << "Invalid MessageType wrt M2C" << std::endl;
	}
}

void Logic::ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient> pM2OC)
{
	switch (pM2OC->messagetype())
	{
	case Protobuf::MessageType::ValidPlayer:
		std::cout << "Valid player." << std::endl;
		break;
	case Protobuf::MessageType::InvalidPlayer:
		sw_AI = false;
		UnBlockAI();
		std::cout << "Invalid player!" << std::endl;
		break;
	case Protobuf::MessageType::Send:
		MessageStorage.push(pM2OC->message());
		break;
	default:
		std::cout << "Invalid MessageType wrt M2OC" << std::endl;
	}
}

void Logic::load(std::shared_ptr<Protobuf::MessageToClient> pM2C)
{
	{
		//首先load到buffer
		std::lock_guard<std::mutex> lck(mtx_buffer);
		pBuffer->characters.clear();
		pBuffer->walls.clear();
		pBuffer->props.clear();
		pBuffer->bullets.clear();
		pBuffer->birthpoints.clear();
#ifdef _COLOR_MAP_BY_HASHING_
		pBuffer->cellColors.clear();
#endif // _COLOR_MAP_BY_HASHING_
		pBuffer->teamScore = pM2C->teamscore();
		pBuffer->selfTeamColor = (THUAI4::ColorType)pM2C->selfteamcolor();
		pBuffer->self = obj2C(pM2C->selfinfo());
		int selfX = pBuffer->self->x;
		int selfY = pBuffer->self->y;

		for (auto it : pM2C->gameobjs())
		{
			if (
#ifdef _ALL_VISIBLE_
				true
#else
				visible(selfX, selfY, it)
#endif // _ALL_VISIBLE_
				)
			{
				switch (it.gameobjtype())
				{
				case Protobuf::GameObjType::Character:
					pBuffer->characters.push_back(obj2C(it));
					break;
				case Protobuf::GameObjType::Wall:
					pBuffer->walls.push_back(obj2W(it));
					break;
				case Protobuf::GameObjType::Prop:
					pBuffer->props.push_back(obj2P(it));
					break;
				case Protobuf::GameObjType::Bullet:
					pBuffer->bullets.push_back(obj2Blt(it));
					break;
				case Protobuf::GameObjType::BirthPoint:
					pBuffer->birthpoints.push_back(obj2Bp(it));
					break;
				default:
					std::cout << "Unknown GameObjType:" << (int)it.gameobjtype() << std::endl;
				}
			}
		}

		for (int i = 0; i < StateConstant::nCells; i++)
		{
			for (int j = 0; j < StateConstant::nCells; j++)
			{
				if (
#ifdef _ALL_VISIBLE_
					true
#else
					CellColorVisible(selfX, selfY, i, j)
#endif
					)
				{
#ifdef _COLOR_MAP_BY_HASHING_
					pBuffer->cellColors.insert(std::pair((i << 16) + j, (THUAI4::ColorType)pM2C->cellcolors(i).rowcolors(j)));
#else
					pBuffer->cellColors[i][j] = (THUAI4::ColorType)pM2C->cellcolors(i).rowcolors(j);
#endif // _COLOR_MAP_BY_HASHING_
				}
#ifndef _COLOR_MAP_BY_HASHING_
				//unorderer_map
				else
				{
					pBuffer->cellColors[i][j] = THUAI4::ColorType::Invisible;
				}
#endif // _COLOR_MAP_BY_HASHING_
			}
		}

		FlagBufferUpdated = true;
		counter_buffer += 1;

		//如果这时候state还没被player访问，就把buffer转到state
		if (mtx_state.try_lock())
		{
			Update();
			mtx_state.unlock();
		}
	}
	cv_buffer.notify_one();
}

void Logic::ProcessMessage(Pointer2Message p2M)
{
	switch (p2M.index())
	{
	case 0: //M2C
		ProcessM2C(std::get<std::shared_ptr<Protobuf::MessageToClient>>(p2M));
		break;
	case 1: //M2OC
		ProcessM2OC(std::get<std::shared_ptr<Protobuf::MessageToOneClient>>(p2M));
		break;
	default:
		std::cout << "std::variant_nops\n";
	}
}

void Logic::UnBlockMtxBufferUpdated()
{
	{
		std::lock_guard<std::mutex> lck(mtx_buffer);
		FlagBufferUpdated = true;
	}
	cv_buffer.notify_one();
}

void Logic::UnBlockAI()
{
	{
		std::lock_guard<std::mutex> lck(mtx_ai);
		WhetherToStartKnown = true;
	}
	cv_ai.notify_one();
}

void Logic::Update()
{
	State* temp = pState;
	pState = pBuffer;
	pBuffer = temp;
	FlagBufferUpdated = false;
	counter_state = counter_buffer;
	CurrentStateAccessed = false;
}

void Logic::PlayerWrapper(std::function<void()> player)
{
	{
		std::unique_lock<std::mutex> lock(mtx_ai);
		cv_ai.wait(lock, [this]() {return WhetherToStartKnown; });
	}
	while (sw_AI)
	{
		player();
	}
}


void Logic::Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType, CreateAIFunc f, int debuglevel, std::string filename)
{
	//初始化

	pAI = f();

	//这里也把处理消息的逻辑挂载进去
	pComm = std::make_unique<Comm>(
		[this](Pointer2Message p2M)
		{
			ProcessMessage(p2M);
		},
		[this, playerID, teamID, jobType]()
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			Protobuf::MessageToServer message;
			message.set_messagetype(Protobuf::MessageType::AddPlayer);
			message.set_playerid(playerID);
			message.set_teamid(teamID);
			message.set_jobtype((Protobuf::JobType)jobType);
			pComm->Send(message);
		},
			[this]()
		{
			sw_AI = false;
			UnBlockMtxBufferUpdated();
		}
		);

	std::ofstream OutFile;
	{
		//又臭又长
		std::function<void()> tu = [this]()
		{
			if (mtx_buffer.try_lock())
			{
				if (FlagBufferUpdated) Update();
				mtx_buffer.unlock();
			}
		};
		std::function<void()> wait = [this]()
		{
			std::unique_lock<std::mutex> lck_buffer(mtx_buffer);
			cv_buffer.wait(lck_buffer, [this]() { return FlagBufferUpdated; });
			Update();
		};
		if (asynchronous)
		{
			if (!debuglevel)
			{
				pApi = std::make_unique<API<true>>([this, playerID, teamID](Protobuf::MessageToServer& M2C) {M2C.set_playerid(playerID); M2C.set_teamid(teamID); return pComm->Send(M2C); },
					[this]() { return MessageStorage.empty(); },
					[this](std::string& s) { return MessageStorage.try_pop(s); }, [this]() { return counter_state; },
					(const State*&)pState, mtx_state, tu, wait);
			}
			else
			{
				bool flag = filename == "";
				if (!flag)
				{
					OutFile.open(filename);
					if (OutFile.fail())
					{
						std::cout << "Failed to open the file " << filename << std::endl;
						flag = true;
					}
				}
				pApi = std::make_unique<DebugApi<true>>([this, playerID, teamID](Protobuf::MessageToServer& M2C) {M2C.set_playerid(playerID); M2C.set_teamid(teamID); return pComm->Send(M2C); },
					[this]() { return MessageStorage.empty(); },
					[this](std::string& s) { return MessageStorage.try_pop(s); }, [this]() { return counter_state; },
					(const State*&)pState, mtx_state, tu, wait, debuglevel != 1,
					flag ? std::cout : OutFile);
			}
		}
		else
		{
			if (!debuglevel)
			{
				pApi = std::make_unique<API<false>>([this, playerID, teamID](Protobuf::MessageToServer& M2C) {M2C.set_playerid(playerID); M2C.set_teamid(teamID); return pComm->Send(M2C); },
					[this]() { return MessageStorage.empty(); },
					[this](std::string& s) { return MessageStorage.try_pop(s); }, [this]() { return counter_state; },
					(const State*&)pState, mtx_state, tu, wait);
			}
			else
			{
				bool flag = filename == "";
				if (!flag)
				{
					OutFile.open(filename);
					if (OutFile.fail())
					{
						std::cout << "Failed to open the file " << filename << std::endl;
						flag = true;
					}
				}
				pApi = std::make_unique<DebugApi<false>>([this, playerID, teamID](Protobuf::MessageToServer& M2C) {M2C.set_playerid(playerID); M2C.set_teamid(teamID); return pComm->Send(M2C); },
					[this]() { return MessageStorage.empty(); },
					[this](std::string& s) { return MessageStorage.try_pop(s); }, [this]() { return counter_state; },
					(const State*&)pState, mtx_state, tu, wait, debuglevel != 1,
					flag ? std::cout : OutFile);
			}
		}

	}

	//启动AI线程（但要等到游戏开始才会正式执行）
	std::thread tAI(&Logic::PlayerWrapper, this,
		asynchronous ?
		(std::function<void()>)[this]() {
			//异步似乎反而逻辑变简洁了
			pApi->StartTimer();
			pAI->play(*pApi);
			pApi->EndTimer();
		}
		:
		(std::function<void()>)[this]() {
			std::lock_guard<std::mutex> lck_state(mtx_state);
			if (!CurrentStateAccessed)
			{	
				CurrentStateAccessed = true;
				pApi->StartTimer();//再细一些的分类可以把这里去掉，但似乎没太大意义
				pAI->play(*pApi);
				pApi->EndTimer();
				
			}
			else
			{
				std::unique_lock<std::mutex> lck_buffer(mtx_buffer);
				//如果buffer没更新就等
				cv_buffer.wait(lck_buffer, [this]() { return FlagBufferUpdated; });
				Update();
			}
		}
		);

	//试图连接Agent
	if (!pComm->Start(address, port)) {
		sw_AI = false;
		UnBlockAI();
		tAI.join();
		OutFile.close();
		return;
	}
	std::cout << "Connect to the agent successfully." << std::endl;

	tAI.join();
	OutFile.close();
	pComm->Join();
}

Logic::Logic() : pState(storage), pBuffer(storage + 1)
{
}

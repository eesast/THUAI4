#include "Logic.h"
#include <fstream>
//#define _ALL_VISIBLE_

Logic::Logic() :pState(storage), pBuffer(storage + 1),
capi([this]() {OnConnect(); }, [this]() {OnClose(); }, [this]() {OnReceive(); })
{
	MessageStorage.clear();
}

Logic::~Logic() {}

bool Logic::visible(int32_t x, int32_t y, Protobuf::GameObjInfo& g)
{
	Protobuf::PropType pT = g.proptype();
	if (g.islaid()
		&& (pT == Protobuf::PropType::Attenuator
			|| pT == Protobuf::PropType::Dirt
			|| pT == Protobuf::PropType::Divider)) return false;

	int64_t dx = x - g.x();
	int64_t dy = y - g.y();
	uint64_t distanceSquared = dx * dx + dy * dy;
	return distanceSquared <= Constants::Map::sightRadiusSquared;

}


void Logic::OnReceive()
{
	cvOnReceive.notify_one();
}
void Logic::OnClose()
{

#ifdef _ENABLE_RECONNECTION_

#else
	{
		std::lock_guard<std::mutex> lck(mtx_game);
		UnexpectedlyClosed = true;
	}
	cv_game.notify_one();

#endif 

}
void Logic::OnConnect()
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::AddPlayer);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	message.set_jobtype((Protobuf::JobType)jobType);
	capi.Send(message);
}

std::shared_ptr<THUAI4::Character> Logic::obj2C(const Protobuf::GameObjInfo& goi)
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
std::shared_ptr<THUAI4::Wall> Logic::obj2W(const Protobuf::GameObjInfo& goi)
{
	std::shared_ptr<THUAI4::Wall> wall = std::make_shared<THUAI4::Wall>();
	wall->guid = goi.guid();
	wall->radius = goi.radius();
	wall->shapeType = (THUAI4::ShapeType)goi.shapetype();
	wall->x = goi.x();
	wall->y = goi.y();
	return wall;
}
std::shared_ptr<THUAI4::Prop> Logic::obj2P(const Protobuf::GameObjInfo& goi)
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
std::shared_ptr<THUAI4::Bullet> Logic::obj2Blt(const Protobuf::GameObjInfo& goi)
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
std::shared_ptr<THUAI4::BirthPoint> Logic::obj2Bp(const Protobuf::GameObjInfo& goi)
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

void Logic::ProcessM2C(std::shared_ptr<Protobuf::MessageToClient> pM2C)
{
	switch (pM2C->messagetype()) {
	case Protobuf::MessageType::StartGame:
		//首先load到buffer
		load(pM2C);//第一帧AI线程还没开始 加载到buffer然后交换指针
		{
			std::lock_guard<std::mutex> lck(mtx_game);
			gamePhase = GamePhase::Gaming;
		}
		cv_game.notify_one();
		break;
	case Protobuf::MessageType::Gaming:
		load(pM2C);
		break;

	case Protobuf::MessageType::EndGame:
	{
		std::lock_guard<std::mutex> lck(mtx_game);
		gamePhase = GamePhase::GameOver;
	}
	cv_game.notify_one();
	break;

	default:
		std::cout << "Invalid MessageType wrt M2C" << std::endl;
	}
}

void Logic::ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient> pM2OC)
{
	switch (pM2OC->messagetype()) {
	case Protobuf::MessageType::ValidPlayer:
	{
		std::lock_guard<std::mutex> lck(mtx_game);
		validity = Validity::Valid;
	}
	cv_game.notify_one();
	break;
	case Protobuf::MessageType::InvalidPlayer:
	{
		std::lock_guard<std::mutex> lck(mtx_game);
		validity = Validity::Invalid;
	}
	cv_game.notify_one();
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

		for (auto it : pM2C->gameobjs()) {
			if (
#ifdef _ALL_VISIBLE_
				true
#else
				visible(selfX, selfY, it)
#endif // _ALL_VISIBLE_			
				) {
				switch (it.gameobjtype()) {
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

		for (int i = 0; i < pM2C->playerguids_size() && pBuffer->playerGUIDs.size(); i++) {
			auto temp = pM2C->playerguids(i);
			for (int j = 0; j < temp.teammateguids_size() && pBuffer->playerGUIDs[i].size(); j++) {
				pBuffer->playerGUIDs[i][j] = static_cast<int32_t>(pM2C->playerguids(i).teammateguids(j));
			}
		}

		for (int i = 0; i < StateConstant::nCells; i++) {
			for (int j = 0; j < StateConstant::nCells; j++) {
				if (
#ifdef _ALL_VISIBLE_
					true
#else
					CellColorVisible(selfX, selfY, i, j)
#endif 		
					) {
#ifdef _COLOR_MAP_BY_HASHING_
					pBuffer->cellColors.insert(std::pair((i << 16) + j, (THUAI4::ColorType)pM2C->cellcolors(i).rowcolors(j)));
#else
					pBuffer->cellColors[i][j] = (THUAI4::ColorType)pM2C->cellcolors(i).rowcolors(j);
#endif // _COLOR_MAP_BY_HASHING_


				}
#ifndef _COLOR_MAP_BY_HASHING_
				//unorderer_map
				else {
					pBuffer->cellColors[i][j] = THUAI4::ColorType::Invisible;
				}
#endif // _COLOR_MAP_BY_HASHING_
			}
		}

		BufferUpdated = true;

		//如果这时候state还没被player访问，就把buffer转到state
		if (mtx_state.try_lock()) {
			State* temp = pState;
			pState = pBuffer;
			pBuffer = temp;
			BufferUpdated = false;
			CurrentStateAccessed = false;
			mtx_state.unlock();
		}
	}
	cv_buffer.notify_one();
}

void Logic::ProcessMessage()
{
	std::unique_lock<std::mutex> lock_game(mtx_game);
	Pointer2Message p2M;
	while (gamePhase != GamePhase::GameOver && !UnexpectedlyClosed && validity != Validity::Invalid) {
		lock_game.unlock();

		//无消息处理时停下来少占资源

		{
			std::unique_lock<std::mutex> lck(mtxOnReceive);//OnReceive里往队列里Push时也锁了
			lock_game.lock();
			while (capi.IsEmpty() && !UnexpectedlyClosed) {//否则在这断线就会锁住
				lock_game.unlock();
				cvOnReceive.wait(lck);
				lock_game.lock();
			}
			lock_game.unlock();
		}
		//std::cout << "ProcessMessage有消息处理" << std::endl;
		if (!capi.TryPop(p2M)) {
			std::cout << "Failed to pop the message\n";
			lock_game.lock();
			continue;
		}

		//处理消息
		switch (p2M.index()) {
		case 0://M2C
			ProcessM2C(std::get<std::shared_ptr<Protobuf::MessageToClient>>(p2M));
			break;
		case 1://M2OC
			ProcessM2OC(std::get<std::shared_ptr<Protobuf::MessageToOneClient>>(p2M));
			break;
		default:
			std::cout << "std::variant_nops\n";
		}

		lock_game.lock();
	}
	lock_game.unlock();
	std::cout << "PM thread terminates" << std::endl;
}

void Logic::PlayerWrapper()
{
	//while判断时保证gamePhase和UnexpectedlyClosed不被其他线程访问
	std::unique_lock<std::mutex> lock_game(mtx_game);

	while (gamePhase == GamePhase::Gaming && !UnexpectedlyClosed) {
		lock_game.unlock();

		std::lock_guard<std::mutex> lck_state(mtx_state);
		if (!CurrentStateAccessed) {
			pApi->StartTimer();
			pAI->play(*pApi);
			pApi->EndTimer();
			CurrentStateAccessed = true;
		}
		else {
			//否则看buffer是否有更新，更新的前提是buffer没被占用
			//所以这里堵塞是可以接受的
			std::unique_lock<std::mutex> lck_buffer(mtx_buffer);
			if (BufferUpdated) {
				State* temp = pState;
				pState = pBuffer;
				pBuffer = temp;
				CurrentStateAccessed = false;
				BufferUpdated = false;
			}
			else {//如果当前state已经接触过且buffer没更新，那就等到buffer更新

				  //意外断线这里也会锁住
				lock_game.lock();
				while (!BufferUpdated && !UnexpectedlyClosed && gamePhase != GamePhase::GameOver) {
					lock_game.unlock();
					cv_buffer.wait(lck_buffer);
					lock_game.lock();
				}
				lock_game.unlock();

				State* temp = pState;
				pState = pBuffer;
				pBuffer = temp;
				CurrentStateAccessed = false;
				BufferUpdated = false;
			}
		}

		lock_game.lock();
	}
	lock_game.unlock();
	std::cout << "AI thread terminates" << std::endl;
}

void Logic::Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType, CreateAIFunc f, int debuglevel, std::string filename)
{
	this->playerID = playerID;
	this->teamID = teamID;
	this->jobType = jobType;
	this->pAI = f();

	std::ofstream OutFile;

	if (!debuglevel) {
		this->pApi = std::make_shared<API>([this](Protobuf::MessageToServer& M2C) {M2C.set_playerid(this->playerID); M2C.set_teamid(this->teamID); capi.Send(M2C); },
			[this]() {return MessageStorage.empty(); },
			[this](std::string& s) {return MessageStorage.try_pop(s); },
			(const State*&)pState);
	}
	else {
		if (filename == "") {
			this->pApi = std::make_shared <DebugApi>([this](Protobuf::MessageToServer& M2C) {M2C.set_playerid(this->playerID); M2C.set_teamid(this->teamID); capi.Send(M2C); },
				[this]() {return MessageStorage.empty(); },
				[this](std::string& s) {return MessageStorage.try_pop(s); },
				(const State*&)pState, debuglevel != 1);
		}
		else {
			OutFile.open(filename);
			if (OutFile.fail()) {
				std::cout << "Failed to open the file " << filename << std::endl;
				return;
			}
			this->pApi = std::make_shared <DebugApi>([this](Protobuf::MessageToServer& M2C) {M2C.set_playerid(this->playerID); M2C.set_teamid(this->teamID); capi.Send(M2C); },
				[this]() {return MessageStorage.empty(); },
				[this](std::string& s) {return MessageStorage.try_pop(s); },
				(const State*&)pState, debuglevel != 1, OutFile);
		}

	}

	//CAPI先连接Agent
	if (!capi.Connect(address, port)) {
		std::cout << "无法连接到Agent" << std::endl;
		capi.Stop();
		OutFile.close();
		return;
	}
	std::cout << "成功连接到Agent" << std::endl;

	{
		//OnConnect() 一连上CAPI就发AddPlayer
		//等待Server发 ValidPlayer GameStart
		//饼：InvalidPlayer补救一下
		std::unique_lock<std::mutex> lck(mtx_game);

		std::thread tPM(&Logic::ProcessMessage, this);

		while (validity == Validity::Unknown && !UnexpectedlyClosed)
			cv_game.wait(lck);

		if (UnexpectedlyClosed) {
			std::cout << "Connection was unexpectedly closed.\n";
			lck.unlock();
			cvOnReceive.notify_one();//否则PM线程会一直等
			tPM.join();
			OutFile.close();
			return;
		}
		if (validity == Validity::Valid) {
			std::cout << "Valid player." << std::endl;
		}
		else {
			std::cout << "Invalid player!" << std::endl;
			lck.unlock();
			cvOnReceive.notify_one();
			tPM.join();
			OutFile.close();
			return;
		}

		while (gamePhase == GamePhase::Uninitialized && !UnexpectedlyClosed)
			cv_game.wait(lck);

		if (UnexpectedlyClosed) {
			std::cout << "Connection was unexpectedly closed.\n";
			lck.unlock();
			cvOnReceive.notify_one();
			tPM.join();
			OutFile.close();
			return;
		}

		std::cout << "游戏开始" << std::endl;
		std::thread tAI(&Logic::PlayerWrapper, this);


		while (gamePhase != GamePhase::GameOver && !UnexpectedlyClosed)
			cv_game.wait(lck);
		if (UnexpectedlyClosed) {
			std::cout << "Connection was unexpectedly closed.\n";
			lck.unlock();
			cvOnReceive.notify_one();
			cv_buffer.notify_one();
			tPM.join();
			tAI.join();
			OutFile.close();
			return;
		}
		std::cout << "Game ends\n";

		lck.unlock();
		cv_buffer.notify_one();
		tPM.join();
		tAI.join();
		OutFile.close();
	}
}

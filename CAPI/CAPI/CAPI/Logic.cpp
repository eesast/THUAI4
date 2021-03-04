#include "Logic.h"

//#define _ALL_VISIBLE_

Logic::Logic() :\
pState(storage),
pBuffer(storage + 1),
capi(playerID,
	teamID,
	jobType,
	mtxOnReceive,
	cvOnReceive,
	[this]() {OnClose(); }),
	api(playerID,
		teamID,
		[this](const Protobuf::MessageToServer& M2C) {capi.Send(M2C); },
		pState, AddMessage) {};

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
	return distanceSquared <= Constants::SightRadiusSquared;

}


inline bool Logic::CellColorVisible(int32_t x, int32_t y, int32_t CellX, int32_t CellY)
{
	int32_t centerX = CellX * Constants::numOfGridPerCell + (Constants::numOfGridPerCell >> 1);
	int32_t centerY = CellY * Constants::numOfGridPerCell + (Constants::numOfGridPerCell >> 1);
	int32_t dx = std::abs(centerX - x);
	int32_t dy = std::abs(centerY - y);
	int32_t D = (Constants::numOfGridPerCell >> 1) + Constants::SightRadius;
	return dx <= D && dy <= D;
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
	birthpoint->shapeType = (THUAI4::ShapeType) goi.shapetype();
	birthpoint->teamID = static_cast<uint16_t>(goi.teamid());
	birthpoint->x = goi.x();
	birthpoint->y = goi.y();
	return birthpoint;
}

void Logic::ProcessM2C(std::shared_ptr<Protobuf::MessageToClient> pM2C)
{
	switch (pM2C->messagetype()) {
	case Protobuf::MessageType::StartGame:
		//����load��buffer
		load(pM2C);//��һ֡AI�̻߳�û��ʼ ���ص�bufferȻ�󽻻�ָ��
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
		AddMessage(pM2OC->message());
		break;
	default:
		std::cout << "Invalid MessageType wrt M2OC" << std::endl;
	}
}


void Logic::load(std::shared_ptr<Protobuf::MessageToClient> pM2C)
{
	{
		//����load��buffer
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

		for (int i = 0; i < (int)pBuffer->playerGUIDs.size(); i++) {
			for (int j = 0; j < (int)pBuffer->playerGUIDs[i].size(); j++) {
				pBuffer->playerGUIDs[i][j] = static_cast<int32_t>(pM2C->playerguids(i).teammateguids(j));
			}
		}

		for (int i = 0; i < THUAI4::State::nCells; i++) {
			for (int j = 0; j < THUAI4::State::nCells; j++) {
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

		//�����ʱ��state��û��player���ʣ��Ͱ�bufferת��state
		if (mtx_state.try_lock()) {
			THUAI4::State* temp = pState;
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

		//����Ϣ����ʱͣ������ռ��Դ

		{
			std::unique_lock<std::mutex> lck(mtxOnReceive);//OnReceive����������PushʱҲ����
			lock_game.lock();
			while (capi.IsEmpty() && !UnexpectedlyClosed) {//����������߾ͻ���ס
				lock_game.unlock();
				cvOnReceive.wait(lck);
				lock_game.lock();
			}
			lock_game.unlock();
		}
		//std::cout << "ProcessMessage����Ϣ����" << std::endl;
		if (!capi.TryPop(p2M)) {
			std::cout << "Failed to pop the message\n";
			lock_game.lock();
			continue;
		}

		//������Ϣ
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
	//while�ж�ʱ��֤gamePhase��UnexpectedlyClosed���������̷߳���
	std::unique_lock<std::mutex> lock_game(mtx_game);

	while (gamePhase == GamePhase::Gaming && !UnexpectedlyClosed) {
		lock_game.unlock();

		std::lock_guard<std::mutex> lck_state(mtx_state);
		if (!CurrentStateAccessed) {
			pAI->play(api);
			CurrentStateAccessed = true;
		}
		else {
			//����buffer�Ƿ��и��£����µ�ǰ����bufferû��ռ��
			//������������ǿ��Խ��ܵ�
			std::unique_lock<std::mutex> lck_buffer(mtx_buffer);
			if (BufferUpdated) {
				THUAI4::State* temp = pState;
				pState = pBuffer;
				pBuffer = temp;
				CurrentStateAccessed = false;
				BufferUpdated = false;
			}
			else {//�����ǰstate�Ѿ��Ӵ�����bufferû���£��Ǿ͵ȵ�buffer����

				  //�����������Ҳ����ס
				lock_game.lock();
				while (!BufferUpdated && !UnexpectedlyClosed && gamePhase != GamePhase::GameOver) {
					lock_game.unlock();
					cv_buffer.wait(lck_buffer);
					lock_game.lock();
				}
				lock_game.unlock();

				THUAI4::State* temp = pState;
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


void Logic::Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType, CreateAIFunc f)
{
	this->playerID = playerID;
	this->teamID = teamID;
	this->jobType = jobType;
	this->pAI = f();
	//CAPI������Agent
	if (!capi.Connect(address, port)) {
		std::cout << "�޷����ӵ�Agent" << std::endl;
		capi.Stop();
		return;
	}
	std::cout << "�ɹ����ӵ�Agent" << std::endl;

	{
		//OnConnect() һ����CAPI�ͷ�AddPlayer
		//�ȴ�Server�� ValidPlayer GameStart
		//����InvalidPlayer����һ��
		std::unique_lock<std::mutex> lck(mtx_game);

		std::thread tPM(&Logic::ProcessMessage, this);

		while (validity == Validity::Unknown && !UnexpectedlyClosed)
			cv_game.wait(lck);

		if (UnexpectedlyClosed) {
			std::cout << "Connection was unexpectedly closed.\n";
			lck.unlock();
			cvOnReceive.notify_one();//����PM�̻߳�һֱ��
			tPM.join();
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
			return;
		}

		while (gamePhase == GamePhase::Uninitialized && !UnexpectedlyClosed)
			cv_game.wait(lck);

		if (UnexpectedlyClosed) {
			std::cout << "Connection was unexpectedly closed.\n";
			lck.unlock();
			cvOnReceive.notify_one();
			tPM.join();
			return;
		}

		std::cout << "��Ϸ��ʼ" << std::endl;
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
			return;
		}
		std::cout << "Game ends\n";

		lck.unlock();
		cv_buffer.notify_one();
		tPM.join();
		tAI.join();
	}
}
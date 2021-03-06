#ifndef LOGIC_H

#define LOGIC_H

#include"proto/Message2Client.pb.h"
#include<functional>
#include<array>
#include<thread>
#include<mutex>
#include<iostream>
#include"Constants.h"
#include"Base.h"
#include"CAPI.h"
#include"API.h"

class Logic {
private:
	//Logic control
	bool UnexpectedlyClosed = false;
	bool BufferUpdated = false;
	bool CurrentStateAccessed = false;

	enum class GamePhase : unsigned char {
		Uninitialized = 0,
		Gaming = 1,
		GameOver = 2,
	};
	GamePhase gamePhase = GamePhase::Uninitialized;

	enum class Validity : unsigned char {

		Unknown = 0,
		Valid = 1,
		Invalid = 2
	};
	Validity validity = Validity::Unknown;

	std::mutex mtxOnReceive;
	std::condition_variable cvOnReceive;
	std::mutex mtx_buffer;
	std::mutex mtx_state;
	std::condition_variable cv_buffer;
	std::mutex mtx_game;
	std::condition_variable cv_game;


	//Game data
	THUAI4::JobType jobType = THUAI4::JobType::Job0;
	int32_t playerID = 0;
	int32_t teamID = 0;

	State* pState;
	State* pBuffer;
	State storage[2];
	concurrency::concurrent_queue<std::string> MessageStorage;

	CAPI capi;
	std::shared_ptr<LogicInterface> pApi;
	std::shared_ptr<AIBase> pAI;

	static bool visible(int32_t x, int32_t y, Protobuf::GameObjInfo&);
	static std::shared_ptr<THUAI4::Character> obj2C(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr<THUAI4::Wall> obj2W(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr<THUAI4::Prop> obj2P(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr<THUAI4::Bullet> obj2Blt(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr<THUAI4::BirthPoint> obj2Bp(const Protobuf::GameObjInfo& goi);
	void ProcessM2C(std::shared_ptr<Protobuf::MessageToClient>);
	void ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient>);

	void OnClose();
	void OnReceive();
	void OnConnect();
	void load(std::shared_ptr<Protobuf::MessageToClient>);//降收到的M2C加载到buffer

	void ProcessMessage();
	void PlayerWrapper();
public:
	Logic();
	~Logic();
	void Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType, CreateAIFunc f, int debuglevel,std::string filename="");

};

#endif	//!LOGIC_H

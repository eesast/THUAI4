#pragma once

#include"proto/Message2Client.pb.h"
#include<list>
#include<array>
#include<thread>
#include<mutex>
#include<iostream>
#include"Constants.h"
#include"AI.h"
#include"CAPI.h"
#include"Structures.h"

class Logic {
private:
	bool UnexpectedlyClosed = false;//用于标示游戏结束前意外断线
	//buffer和state的状态
	bool BufferUpdated = false;
	bool CurrentStateAccessed = false;

	enum GamePhase :unsigned char {
		Uninitialized = 0,
		Gaming = 1,
		GameOver = 2,
	};
	GamePhase gamePhase = Uninitialized;

	enum Validity :unsigned char {
		Unknown = 0,
		Valid = 1,
		Invalid = 2
	};
	Validity validity = Unknown;

	//state buffer分别指向storage的两个区域，信息收到以后直接写给buffer
	//不能与state buffer换位同时进行

	THUAI4::State* pState;
	THUAI4::State* pBuffer;

	std::mutex mtx_buffer;
	std::mutex mtx_state;
	std::condition_variable cv_buffer;

	//游戏结点的控制
	std::mutex mtx_game;
	std::condition_variable cv_game;

	CAPI capi;
	AI ai;

	THUAI4::State storage[2];//团团转

	//一些辅助函数
	static bool visible(int32_t x, int32_t y, Protobuf::GameObjInfo&);//饼
	static std::shared_ptr<THUAI4::Character> obj2C(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::Wall> obj2W(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::Prop> obj2P(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::Bullet> obj2Blt(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::BirthPoint> obj2Bp(const Protobuf::GameObjInfo& goi);
	void ProcessM2C(std::shared_ptr<Protobuf::MessageToClient>);
	void ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient>);

	void load(std::shared_ptr<Protobuf::MessageToClient>);//将收到的M2C加载到buffer
public:
	Logic();
	void Main(const char* address, USHORT port, int32_t playerID, int32_t teamID, Protobuf::JobType jobType);
	void ProcessMessage();
	void PlayerWrapper();
	friend class API;
	friend class CAPI;
};

#pragma once

#ifndef LOGIC_H

#define LOGIC_H

#include "proto/Message2Client.pb.h"
#include <functional>
#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include "Base.h"
#include "CAPI.h"
#include "API.h"

class Logic
{
private:
	//Logic control
	bool FlagProcessMessage = false;
	bool FlagBufferUpdated = false;
	bool CurrentStateAccessed = false;
	bool AiTerminated = true;//更确切的含义是，AI线程是否终止或未开始

	enum class GamePhase : unsigned char
	{
		Uninitialized = 0,
		Gaming = 1,
		GameOver = 2,
	};
	std::atomic<GamePhase> gamePhase = GamePhase::Uninitialized; //仅用于循环条件判断，atomic即可
	volatile std::int32_t counter_state = 0;
	volatile std::int32_t counter_buffer = 0;

	std::mutex mtxOnReceive;
	std::condition_variable cvOnReceive;
	std::mutex mtx_buffer;
	std::mutex mtx_state;
	std::condition_variable cv_buffer;

	std::mutex mtx_ai;
	std::condition_variable cv_ai;

	//Game data
	THUAI4::JobType jobType = THUAI4::JobType::Job0;
	int32_t playerID = 0;
	int32_t teamID = 0;

	State* pState;
	State* pBuffer;
	State storage[2];
	concurrency::concurrent_queue<std::string> MessageStorage;
	concurrency::concurrent_queue<Pointer2Message> queue;

	CAPI capi;
	std::unique_ptr<LogicInterface> pApi;
	std::unique_ptr<AIBase> pAI;

	void UnBlockMtxOnReceive();
	void UnBlockMtxBufferUpdated();

	void ProcessM2C(std::shared_ptr<Protobuf::MessageToClient>);
	void ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient>);

	void load(std::shared_ptr<Protobuf::MessageToClient>); //将最新状态信息加载到buffer 还会使得counter_buffer计数加一

	void ProcessMessage();
	void PlayerWrapper();
	void PlayerWrapperAsyn();

public:
	Logic();
	~Logic() = default;
	void Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType, CreateAIFunc f, int debuglevel, std::string filename = "");
};

#endif //!LOGIC_H

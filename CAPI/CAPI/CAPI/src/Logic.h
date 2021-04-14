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
	using Pointer2Message = std::variant<std::shared_ptr<Protobuf::MessageToClient>, std::shared_ptr<Protobuf::MessageToOneClient>>;

	bool AiTerminated = true;//更确切的含义是，AI线程是否终止或未开始
	bool FlagBufferUpdated = false;
	bool CurrentStateAccessed = false;
	std::atomic_bool sw_AI = true;

	std::mutex mtx_ai;
	std::condition_variable cv_ai;
	std::mutex mtx_buffer;
	std::mutex mtx_state;
	std::condition_variable cv_buffer;
	Communication<Protobuf::MessageToServer, 1, Protobuf::MessageToClient, 0, Protobuf::MessageToOneClient, 2> comm;

	void UnBlockMtxBufferUpdated();
	void PlayerWrapper();
	void PlayerWrapperAsyn();
	void ProcessM2C(std::shared_ptr<Protobuf::MessageToClient>);
	void ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient>);
	void ProcessMessage(Pointer2Message);

	//Game data
	THUAI4::JobType jobType = THUAI4::JobType::Job0;
	int32_t playerID = 0;
	int32_t teamID = 0;

	volatile std::int32_t counter_state = 0;
	volatile std::int32_t counter_buffer = 0;

	State* pState;
	State* pBuffer;
	State storage[2];
	concurrency::concurrent_queue<std::string> MessageStorage;

	std::unique_ptr<LogicInterface> pApi;
	std::unique_ptr<AIBase> pAI;

	//将最新状态信息加载到buffer 还会使得counter_buffer计数加一
	void load(std::shared_ptr<Protobuf::MessageToClient>);
	//即pState与pBuffer指向的地址互换
	void Update();
public:
	Logic();
	~Logic() = default;
	void Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType, CreateAIFunc f, int debuglevel, std::string filename = "");
};

#endif //!LOGIC_H

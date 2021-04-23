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
	using Comm = Communication<Protobuf::MessageToServer, 1, Protobuf::MessageToClient, 0, Protobuf::MessageToOneClient, 2>;
	
	std::atomic_bool sw_AI = true;
	bool FlagBufferUpdated = false;
	bool CurrentStateAccessed = false;
	bool WhetherToStartKnown = false;//也许是我设计的问题，无法优雅地让AI线程开始并结束……
	std::mutex mtx_buffer;
	std::mutex mtx_state;
	std::mutex mtx_ai;
	std::condition_variable cv_buffer;//asyn=false情况下若无更新会阻塞
	std::condition_variable cv_ai;

	std::unique_ptr<Comm> pComm;
	std::unique_ptr<LogicInterface> pApi;
	std::unique_ptr<AIBase> pAI;

	volatile std::int32_t counter_state = 0;
	volatile std::int32_t counter_buffer = 0;
	State* pState;
	State* pBuffer;
	State storage[2];
	concurrency::concurrent_queue<std::string> MessageStorage;


	//AI线程执行的函数
	void PlayerWrapper(std::function<void()> player);

	//作为Comm类的构造函数参数，每有消息（非并发）则调用
	void ProcessMessage(Pointer2Message);

	//处理MessageToClient
	void ProcessM2C(std::shared_ptr<Protobuf::MessageToClient>);

	//处理MessageToOneClient
	void ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient>);
	
	//将状态信息加载到buffer，并使counter_buffer加一
	void load(std::shared_ptr<Protobuf::MessageToClient>);

	//即pState与pBuffer指向的地址互换
	void Update();

	//asynchronous=false时的一个辅助函数
	void UnBlockMtxBufferUpdated();

	//辅助函数，知道了AI线程该不该开始后执行
	void UnBlockAI();
public:
	Logic();
	~Logic() = default;
	void Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType, CreateAIFunc f, int debuglevel, std::string filename = "");
};

#endif //!LOGIC_H

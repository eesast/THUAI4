#include "CAPI.h"
#include <iostream>
#include <thread>
#include <chrono>


template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::CAPI(std::function<void()> onconnect, std::function<void()> onclose, std::function<void(Pointer2M)> onreceive) : __OnConnect(onconnect), __OnReceive(onreceive), __OnClose(onclose), pclient(this) {}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
EnHandleResult CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::OnConnect(ITcpClient* pSender, CONNID dwConnID)
{
	__OnConnect();
	return HR_OK;
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
EnHandleResult CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	uint32_t type = (uint32_t)pData[0];
	type |= ((uint32_t)pData[1]) << 8;
	type |= ((uint32_t)pData[2]) << 16;
	type |= ((uint32_t)pData[3]) << 24;
	//type = *((int32_t*)pData);

	Pointer2M p2m;
	if (type == typeM2C1)
	{
		std::shared_ptr<Message2C1> pM2C = std::make_shared<Message2C1>();
		pM2C->ParseFromArray(pData + 4, iLength - 4);
		p2m = pM2C;
	}
	else if (type == typeM2C2)
	{
		std::shared_ptr< Message2C2> pM2OC = std::make_shared< Message2C2>();
		pM2OC->ParseFromArray(pData + 4, iLength - 4);
		p2m = pM2OC;
	}
	else
	{
		std::cout << "Unknown type of message!!!" << std::endl;
		return HR_ERROR;
	}
	__OnReceive(std::move(p2m));
	return HR_OK;
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
EnHandleResult CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	__OnClose();
	return HR_OK;
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
bool CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::Connect(const char* address, uint16_t port)
{
	std::cout << "Connecting......" << std::endl;
	while (!pclient->IsConnected())
	{
		if (!pclient->Start(address, port))
		{
			std::cout << "Failed to connect with the agent" << std::endl;
			return false;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return true;
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
void CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::Send(const Message2S& message)
{
	unsigned char data[maxlength];
	data[0] = typeM2S & 0xff;
	data[1] = (typeM2S >> 8) & 0xff;
	data[2] = (typeM2S >> 16) & 0xff;
	data[3] = (typeM2S >> 24) & 0xff;
	int msgSize = message.ByteSizeLong();
	message.SerializeToArray(data + 4, msgSize);
	if (!pclient->Send(data, 4 + msgSize))
	{
		std::cout << "Failed to send the message. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
void CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::Stop()
{
	if (!pclient->HasStarted()) return;
	if (!pclient->Stop())
	{
		std::cout << "The client wasn`t stopped. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}




template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
void Communication<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::UnBlock()
{
	{
		std::lock_guard<std::mutex> lck(mtx);
		blocking = false;
	}
	cv.notify_one();
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
void Communication<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::ProcessMessage()
{
	Pointer2M p2M;
	while (loop)
	{
		{
			std::unique_lock<std::mutex> lck(mtx);
			blocking = queue.empty();
			cv.wait(lck, [this]() { return !blocking; });
		}
		if (!queue.try_pop(p2M))
		{
			if (loop) std::cout << "Failed to pop the message\n";
			continue;
		}
		__OnReceive(std::move(p2M));
	}
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
bool  Communication<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::Start(const char* address, uint16_t port)
{
	tPM = std::thread(&Communication::ProcessMessage, this); //单线程处理收到的消息
	if (!capi.Connect(address, port))
	{
		std::cout << "无法连接到Agent" << std::endl;
		tPM.join();
		return false;
	}
	return true;
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
bool  Communication<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::Send(const Message2S& m)
{
	if (counter == Limit) return false;
	capi.Send(m);
	counter++;
	return true;
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
void  Communication<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::Join()
{
	capi.Stop();
	loop = false;
	UnBlock();
	tPM.join();
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
Communication<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::~Communication()
{
	capi.Stop();
	loop = false;
	UnBlock();
	if (tPM.joinable()) tPM.join();
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
Communication<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::
Communication(std::function<void(Pointer2M)> OnReceive, std::function<void() > OnConnect, std::function<void() > CloseHandler) :
	__OnReceive(OnReceive), __OnClose(CloseHandler),
	capi(
		OnConnect,
		[this]()
		{
			std::cout << "Connection was closed.\n";
			loop = false;
			UnBlock();
			if (__OnClose != nullptr)
				__OnClose();

		},
		[this](Pointer2M p2M) {
			if (p2M.index() == typeM2C1) counter = 0;
			queue.push(p2M);
			UnBlock();
		})
{}
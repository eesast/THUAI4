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
			std::cout << "Failed to connect with the agent. Error code:";
			std::cout << pclient->GetLastError() << std::endl;
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
	if (pclient->Send(data, 4 + msgSize))
		;
	else
	{
		std::cout << "Failed to send. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
void CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2>::Stop()
{
	if (!pclient->Stop())
	{
		std::cout << "The client wasn`t stopped. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}

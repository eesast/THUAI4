#include "CAPI.h"
#include <iostream>
#include <thread>
#include <chrono>

CAPI::CAPI(std::function<void()> onconnect, std::function<void()> onclose, std::function<void(Pointer2Message)> onreceive) : __OnConnect(onconnect), __OnReceive(onreceive), __OnClose(onclose), pclient(this) {}

EnHandleResult CAPI::OnConnect(ITcpClient *pSender, CONNID dwConnID)
{
	__OnConnect();
	return HR_OK;
}

EnHandleResult CAPI::OnReceive(ITcpClient *pSender, CONNID dwConnID, const BYTE *pData, int iLength)
{
	uint32_t type = (uint32_t)pData[0];
	type |= ((uint32_t)pData[1]) << 8;
	type |= ((uint32_t)pData[2]) << 16;
	type |= ((uint32_t)pData[3]) << 24;
	//type = *((int32_t*)pData);

	Pointer2Message p2m;
	if (type == MessageToClient)
	{
		std::shared_ptr<Protobuf::MessageToClient> pM2C = std::make_shared<Protobuf::MessageToClient>();
		pM2C->ParseFromArray(pData + 4, iLength - 4);
		p2m = pM2C;
	}
	else if (type == MessageToOneClient)
	{
		std::shared_ptr<Protobuf::MessageToOneClient> pM2OC = std::make_shared<Protobuf::MessageToOneClient>();
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

EnHandleResult CAPI::OnClose(ITcpClient *pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	__OnClose();
	return HR_OK;
}

bool CAPI::Connect(const char *address, uint16_t port)
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
void CAPI::Send(const Protobuf::MessageToServer &message)
{
	unsigned char data[maxlength];
	data[0] = MessageToServer & 0xff;
	data[1] = (MessageToServer >> 8) & 0xff;
	data[2] = (MessageToServer >> 16) & 0xff;
	data[3] = (MessageToServer >> 24) & 0xff;
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

void CAPI::Stop()
{
	if (!pclient->Stop())
	{
		std::cout << "The client wasn`t stopped. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}

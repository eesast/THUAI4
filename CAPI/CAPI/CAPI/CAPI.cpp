#include "CAPI.h"
#include "Structures.h"
#include <iostream>

Listener::Listener(std::mutex& mtx, std::condition_variable& cv, std::function<void(Pointer2Message)> push, std::function<void()> onconnect, std::function<void()> onclose) :
	mtxOnReceive(mtx), cvOnReceive(cv), Push(push), OnConnectL(onconnect), OnCloseL(onclose) {}

EnHandleResult Listener::OnConnect(ITcpClient* pSender, CONNID dwConnID)
{
	OnConnectL();
	return HR_OK;
}

EnHandleResult Listener::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
	int32_t type = (int32_t)pData[0];
	type |= ((int32_t)pData[1]) << 8;
	type |= ((int32_t)pData[2]) << 16;
	type |= ((int32_t)pData[3]) << 24;
	//type = *((int32_t*)pData);

	Pointer2Message p2m;
	if (type == Constants::MessageToClient) {
		std::shared_ptr<Protobuf::MessageToClient> pM2C = std::make_shared<Protobuf::MessageToClient>();
		pM2C->ParseFromArray(pData + 4, iLength - 4);
		p2m = pM2C;
	}
	else if (type == Constants::MessageToOneClient) {
		std::shared_ptr<Protobuf::MessageToOneClient> pM2OC = std::make_shared<Protobuf::MessageToOneClient>();
		pM2OC->ParseFromArray(pData + 4, iLength - 4);
		p2m = pM2OC;
	}
	else {
		std::cout << "Unknown type of message!!!" << std::endl;
		return HR_ERROR;
	}

	{
		std::lock_guard<std::mutex> lck(mtxOnReceive);
		Push(p2m);
	}
	cvOnReceive.notify_one();
	return HR_OK;
}

EnHandleResult Listener::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	OnCloseL();
	return HR_OK;
}

CAPI::CAPI(const int32_t& pID,
	const int32_t& tID,
	const THUAI4::JobType& jT,
	std::mutex& mtx,
	std::condition_variable& cv,
	std::function<void()> onclose) :\
	playerID(pID), teamID(tID), jobType(jT),
	listener(mtx, cv,
		[this](Pointer2Message p2M) {Push(p2M); },
		[this]() {OnConnect(); },
		onclose
	), pclient(&listener) {
	queue.clear();
}


bool CAPI::Connect(const char* address, uint16_t port)
{
	std::cout << "Connecting......" << std::endl;
	while (!pclient->IsConnected()) {
		if (!pclient->Start(address, port)) {
			std::cout << "Failed to connect with the agent. Error code:";
			std::cout << pclient->GetLastError() << std::endl;
			return false;
		}
		Sleep(1000);
	}
	return true;

}
void CAPI::Send(const Protobuf::MessageToServer& message)
{
	byte data[Constants::maxlength];
	data[0] = Constants::MessageToServer & 0xff;
	data[1] = (Constants::MessageToServer >> 8) & 0xff;
	data[2] = (Constants::MessageToServer >> 16) & 0xff;
	data[3] = (Constants::MessageToServer >> 24) & 0xff;
	int msgSize = message.ByteSizeLong();
	message.SerializeToArray(data + 4, msgSize);
	if (pclient->Send(data, 4 + msgSize));
	else {
		std::cout << "Failed to send. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}

void CAPI::Stop()
{
	if (pclient->Stop());
	else {
		std::cout << "The client wasn`t stopped. Error code:";
		std::cout << pclient->GetLastError() << std::endl;
	}
}

void CAPI::Push(Pointer2Message ptr)
{

	queue.push(ptr);
}

bool CAPI::TryPop(Pointer2Message& ptr)
{
	if (queue.empty()) return false;
	return queue.try_pop(ptr);
}

bool CAPI::IsEmpty()
{
	return queue.empty();
}

void CAPI::OnConnect()
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::AddPlayer);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	message.set_jobtype((Protobuf::JobType)jobType);
	Send(message);
}


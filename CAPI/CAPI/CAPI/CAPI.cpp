#include "CAPI.h"
#include "Logic.h"
#include <iostream>
#include <cstdint>


EnHandleResult Listener::OnConnect(ITcpClient* pSender, CONNID dwConnID)
{
	pCAPI->OnConnect();
	return HR_OK;
}

EnHandleResult Listener::OnReceive(ITcpClient* pSender, CONNID dwConnID, const uint8_t* pData, int iLength)
{
	int32_t type = (int32_t)pData[0];
	type |= ((int32_t)pData[1]) << 8;
	type |= ((int32_t)pData[2]) << 16;
	type |= ((int32_t)pData[3]) << 24;

	Pointer2Message p2m;
	if (type == Constants::MessageToClient) {
		auto pM2C = std::make_shared<Protobuf::MessageToClient>();
		pM2C->ParseFromArray(pData + 4, iLength - 4);
		p2m = pM2C;
	}
	else if (type == Constants::MessageToOneClient) {
		auto pM2OC = std::make_shared<Protobuf::MessageToOneClient>();
		pM2OC->ParseFromArray(pData + 4, iLength - 4);
		p2m = pM2OC;
	}
	else {
		std::cout << "Unknown type of message!!!" << std::endl;
		return HR_ERROR;
	}

	{
		std::lock_guard<std::mutex> lck(pCAPI->mtx);
		pCAPI->Push(p2m);
	}
	pCAPI->cv.notify_one();
	//std::cout << "Listener OnReceive" << std::endl;
	return HR_OK;
}

EnHandleResult Listener::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{
	pCAPI->OnClose();
	return HR_OK;
}

CAPI::CAPI(Logic& l) : listener(this), pclient(&listener), logic(l)
{
	queue.clear();
}

bool CAPI::Connect(const char* address, unsigned short port)
{
	std::cout << "Connecting......" << std::endl;
	while (!pclient->IsConnected()) {
		if (!pclient->Start((LPCTSTR)address, port)) {
			std::cout << "Failed to connect with the agent.\n Error code:";
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

void CAPI::set_player(int32_t playerID, int32_t teamID, Protobuf::JobType jobType)
{
	this->playerID = playerID;
	this->teamID = teamID;
	this->jobtype = jobType;
}
void CAPI::OnConnect()
{
	Protobuf::MessageToServer message;
	message.set_messagetype(Protobuf::MessageType::AddPlayer);
	message.set_playerid(playerID);
	message.set_teamid(teamID);
	message.set_jobtype(jobtype);
	Send(message);
}
void CAPI::OnClose()
{

#ifdef ENABLE_RECONNECTION//±ý±ý


#else
	{
		std::lock_guard<std::mutex> lck(logic.mtx_game);
		logic.UnexpectedlyClosed = true;
	}
	logic.cv_game.notify_one();

#endif 
	
}


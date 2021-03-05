#pragma once

#ifndef CAPI_H

#define CAPI_H

#include "Constants.h"
#include "Structures.h"
#include "proto/Message2Client.pb.h"
#include "proto/Message2Server.pb.h"
#include "concurrent_queue.hpp"
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <variant>	//c++17

using Pointer2Message = std::variant<std::shared_ptr<Protobuf::MessageToClient>, std::shared_ptr<Protobuf::MessageToOneClient>>;

//index: 0 message2client 1 message2oneclient

class CAPI;
class Listener :public CTcpClientListener
{
private:
	const std::function<void(Pointer2Message)> Push;//不仅push还调用cvOnReceive.notify_one()
	const std::function<void()> OnCloseL;
	const std::function<void()> OnConnectL;
public:
	Listener(std::function<void(Pointer2Message)>, std::function<void()>, std::function<void()>);
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
};

class CAPI
{
private:
	const std::function<void()> OnReceive;//调用cvOnReceive.notify_one()
	concurrency::concurrent_queue<Pointer2Message> queue;
	Listener listener;
	CTcpPackClientPtr pclient;
public:
	CAPI(std::function<void()>, std::function<void()>, std::function<void()>);
	bool Connect(const char* address, uint16_t port);
	void Send(const Protobuf::MessageToServer&);
	void Stop();
	bool TryPop(Pointer2Message&);
	bool IsEmpty();
};

#endif //!CAPI_H

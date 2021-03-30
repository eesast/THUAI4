#pragma once

#ifndef CAPI_H

#define CAPI_H

#include "proto/Message2Client.pb.h"
#include "proto/Message2Server.pb.h"
#include "concurrent_queue.hpp"
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <variant> //c++17

using Pointer2Message = std::variant<std::shared_ptr<Protobuf::MessageToClient>, std::shared_ptr<Protobuf::MessageToOneClient>>;

//index: 0 message2client 1 message2oneclient

class CAPI : public CTcpClientListener
{
private:
	static const int maxlength = 1000;
	static const int32_t MessageToClient = 0;
	static const int32_t MessageToServer = 1;
	static const int32_t MessageToOneClient = 2;
	const std::function<void()> __OnConnect;
	const std::function<void()> __OnReceive;
	const std::function<void()> __OnClose;
	concurrency::concurrent_queue<Pointer2Message> queue;
	CTcpPackClientPtr pclient;

public:
	CAPI(std::function<void()>, std::function<void()>, std::function<void()>);
	bool Connect(const char* address, uint16_t port);
	void Send(const Protobuf::MessageToServer&);
	void Stop();
	bool TryPop(Pointer2Message&);
	bool IsEmpty() const;
	virtual EnHandleResult OnConnect(ITcpClient *pSender, CONNID dwConnID);
	virtual EnHandleResult OnClose(ITcpClient *pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnReceive(ITcpClient *pSender, CONNID dwConnID, const BYTE *pData, int iLength);
};

#endif //!CAPI_H

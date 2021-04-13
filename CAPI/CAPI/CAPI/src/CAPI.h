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
	const std::function<void(Pointer2Message)> __OnReceive;
	const std::function<void()> __OnClose;
	CTcpPackClientPtr pclient;

public:
	// CAPI构造函数
	//onconnect: OnConnect 触发时调用的回调函数
	//onclose: OnClose 触发时调用的回调函数
	//onreceive: OnReceive 触发时调用的回调函数
	CAPI(std::function<void()> onconnect, std::function<void()> onclose, std::function<void(Pointer2Message)> onreceive);
	
	//连接Agent 成功返回真，否则返回假
	//address: Agent IP
	//port: Agent 监听的端口号
	bool Connect(const char* address, uint16_t port);

	//发送消息
	void Send(const Protobuf::MessageToServer&);

	//停止client
	void Stop();

	//HPSocket提供的回调函数接口
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
};

#endif //!CAPI_H

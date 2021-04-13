#pragma once

#ifndef CAPI_H

#define CAPI_H

#include "proto/Message2Client.pb.h"
#include "proto/Message2Server.pb.h"

#include <google/protobuf/message.h>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <variant> 
#include <type_traits>
#include <functional>

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
class CAPI : public CTcpClientListener
{
private:
	static_assert(std::is_base_of<google::protobuf::Message, Message2S>::value);
	static_assert(std::is_base_of<google::protobuf::Message, Message2C1>::value);
	static_assert(std::is_base_of<google::protobuf::Message, Message2C2>::value);
	static_assert(typeM2S != typeM2C1);
	static_assert(typeM2S != typeM2C2);
	static_assert(typeM2C1 != typeM2C2);

	using Pointer2M = std::variant<std::shared_ptr<Message2C1>, std::shared_ptr<Message2C2>>;
	static const int maxlength = 1000;
	const std::function<void()> __OnConnect;
	const std::function<void(Pointer2M)> __OnReceive;
	const std::function<void()> __OnClose;
	CTcpPackClientPtr pclient;


	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);

public:
	// CAPI构造函数
	//onconnect: OnConnect 触发时调用的回调函数 void()
	//onclose: OnClose 触发时调用的回调函数 void()
	//onreceive: OnReceive 触发时调用的回调函数 void(std::variant<std::shared_ptr<Message2C1>, std::shared_ptr<Message2C2>>)
	CAPI(std::function<void()> onconnect, std::function<void()> onclose, std::function<void(Pointer2M)> onreceive);

	//连接Agent 成功返回真，否则返回假
	//address: Agent IP
	//port: Agent 监听的端口号
	bool Connect(const char* address, uint16_t port);

	//发送消息
	void Send(const Message2S&);

	//停止client
	void Stop();

};

template CAPI<Protobuf::MessageToServer, 1, Protobuf::MessageToClient, 0, Protobuf::MessageToOneClient, 2>;
#endif //!CAPI_H

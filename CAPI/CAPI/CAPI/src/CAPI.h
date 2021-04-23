#pragma once

#ifndef CAPI_H

#define CAPI_H

#include "proto/Message2Client.pb.h"
#include "proto/Message2Server.pb.h"
#include"concurrent_queue.hpp"

#include <google/protobuf/message.h>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <variant> 
#include <type_traits>
#include <functional>
#include <thread>
#include <condition_variable>


//CAPI 
//模板参数：发给server的消息及其对应type 发给client的消息及其对应type
//要求：
//消息必须以google::protobuf::Message为基类 type互不相同
//假设收发字节流前四比特是type(little endian)，随后是Message的字节流
//
//由于我不会varadic template 该模板要求发给server的消息有一种，发给client的消息有两种 可以根据需求改

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
class CAPI final : public CTcpClientListener
{
private:
	static_assert(std::is_base_of<google::protobuf::Message, Message2S>::value);
	static_assert(std::is_base_of<google::protobuf::Message, Message2C1>::value);
	static_assert(std::is_base_of<google::protobuf::Message, Message2C2>::value);
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
	//构造函数
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

template class CAPI<Protobuf::MessageToServer, 1, Protobuf::MessageToClient, 0, Protobuf::MessageToOneClient, 2>;

//对CAPI做进一步封装，最主要的变化是保证OnReceive不并发

template<typename Message2S, int typeM2S, typename Message2C1, int typeM2C1, typename Message2C2, int typeM2C2>
class Communication {
private:
	static_assert(std::is_base_of<google::protobuf::Message, Message2S>::value);
	static_assert(std::is_base_of<google::protobuf::Message, Message2C1>::value);
	static_assert(std::is_base_of<google::protobuf::Message, Message2C2>::value);
	static_assert(typeM2C1 != typeM2C2);

	using Pointer2M = std::variant<std::shared_ptr<Message2C1>, std::shared_ptr<Message2C2>>;
	bool blocking = false;//在没有消息时线程阻塞节约资源
	std::atomic_bool loop = true;
	const std::function<void(Pointer2M)> __OnReceive;
	const std::function<void()> __OnClose;

	std::thread tPM;
	std::mutex mtx;
	std::condition_variable cv;

	concurrency::concurrent_queue<Pointer2M> queue;
	CAPI<Message2S, typeM2S, Message2C1, typeM2C1, Message2C2, typeM2C2> capi;
	void UnBlock();
	void ProcessMessage();
public:

	//OnReceive 处理收到的消息，保证不并发
	//OnConnect 连接成功时的回调函数
	//CloseHandler 连接关闭时的回调函数，可选
	//但断线时 Communication 的 PM 进程会自动结束，可在构造函数那里改动
	Communication(std::function<void(Pointer2M)> OnReceive, std::function<void() > OnConnect, std::function<void() > CloseHandler = nullptr);
	~Communication();

	//连接Agent 成功返回真且启动 PM 线程， 否则返回假且不启动线程
	bool Start(const char* address, uint16_t port);

	//发消息
	void Send(const Message2S&);

	//结束线程并释放资源（当然构析函数也可以做到，所以其实没必要）
	void Join();
};

template class Communication<Protobuf::MessageToServer, 1, Protobuf::MessageToClient, 0, Protobuf::MessageToOneClient, 2>;

#endif //!CAPI_H

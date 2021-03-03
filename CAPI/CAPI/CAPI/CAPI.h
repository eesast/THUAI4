#pragma once

#ifndef CAPI_H

#define CAPI_H

#include "Constants.h"
#include "proto/Message2Client.pb.h"
#include "proto/Message2Server.pb.h"
#include <concurrent_queue.h>
#include <HPSocket/HPSocket.h>
#include <HPSocket/SocketInterface.h>
#include <variant>

using Pointer2Message = std::variant<std::shared_ptr<Protobuf::MessageToClient>, std::shared_ptr<Protobuf::MessageToOneClient>>;

//index: 0 message2client 1 message2oneclient
class Logic;

class CAPI;

class Listener : public CTcpClientListener
{
private:

	CAPI* const pCAPI;

public:
	Listener(CAPI* ptr) : pCAPI(ptr) {};
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
};

class CAPI
{
private:

	Protobuf::JobType jobtype;
	int32_t playerID;
	int32_t teamID;
	Logic& logic;
	//�ƺ���ͨ����Ҳ���ԣ�����������
	concurrency::concurrent_queue<Pointer2Message> queue;
	Listener listener;
	CTcpPackClientPtr pclient;

public:

	void set_player(int32_t playerID, int32_t teamID, Protobuf::JobType jobType);

	std::mutex mtx;//���������Ҫ�������ö���Ϊ��ʱProcessMessage����
	//�ƺ�Ҳ�ܷ�ֹͬʱPUSH/PULL ��������о�˵�ǰ�ȫ��
	std::condition_variable cv;
	CAPI(Logic& l);
	void OnConnect();
	void OnClose();
	bool Connect(const char* address, unsigned short port);
	void Send(const Protobuf::MessageToServer&);
	void Stop();
	void Push(Pointer2Message);
	bool TryPop(Pointer2Message&);
	bool IsEmpty();
};

#endif // !CAPI_H

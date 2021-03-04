#pragma once

#include"proto/Message2Client.pb.h"
#include<functional>
#include<array>
#include<thread>
#include<mutex>
#include<iostream>
#include"Constants.h"
#include"AI.h"
#include"CAPI.h"

class Logic {
private:
	bool UnexpectedlyClosed = false;//���ڱ�ʾ��Ϸ����ǰ�������
	//buffer��state��״̬
	bool BufferUpdated = false;
	bool CurrentStateAccessed = false;

	enum class GamePhase :unsigned char {
		Uninitialized = 0,
		Gaming = 1,
		GameOver = 2,
	};
	GamePhase gamePhase = GamePhase::Uninitialized;

	enum class Validity :unsigned char {
		Unknown = 0,
		Valid = 1,
		Invalid = 2
	};
	Validity validity = Validity::Unknown;

	THUAI4::JobType jobType = THUAI4::JobType::Job0;
	int32_t playerID = 0;
	int32_t teamID = 0;

	//state buffer�ֱ�ָ��storage������������Ϣ�յ��Ժ�ֱ��д��buffer
	THUAI4::State* pState;
	THUAI4::State* pBuffer;
	THUAI4::State storage[2];//����ת

	std::function<void(std::string)> AddMessage;//��API�е�MessageStorage Push

	std::mutex mtxOnReceive;
	std::condition_variable cvOnReceive;

	std::mutex mtx_buffer;
	std::mutex mtx_state;
	std::condition_variable cv_buffer;

	//��Ϸ���Ŀ���
	std::mutex mtx_game;
	std::condition_variable cv_game;

	CAPI capi;
	AI ai;


	//һЩ��������
	static bool visible(int32_t x, int32_t y, Protobuf::GameObjInfo&);//��
	static inline bool CellColorVisible(int32_t x, int32_t y, int32_t CellX, int32_t CellY);
	static std::shared_ptr<THUAI4::Character> obj2C(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::Wall> obj2W(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::Prop> obj2P(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::Bullet> obj2Blt(const Protobuf::GameObjInfo& goi);
	static std::shared_ptr <THUAI4::BirthPoint> obj2Bp(const Protobuf::GameObjInfo& goi);
	void ProcessM2C(std::shared_ptr<Protobuf::MessageToClient>);
	void ProcessM2OC(std::shared_ptr<Protobuf::MessageToOneClient>);
	void OnClose();//��֧�ֶ������� ���˾��˳�
	void load(std::shared_ptr<Protobuf::MessageToClient>);//���յ���M2C���ص�buffer
public:
	Logic();
	void Main(const char* address, uint16_t port, int32_t playerID, int32_t teamID, THUAI4::JobType jobType);
	void ProcessMessage();
	void PlayerWrapper();
};

#pragma once
#include"API.h"
class AI :protected API {
public:
	AI(const int32_t& pID,
		const int32_t& tID,
		std::function<void(const Protobuf::MessageToServer&)> f,
		const THUAI4::State* pS) :\
		API(pID, tID, f, pS) {}
	virtual void play();
};



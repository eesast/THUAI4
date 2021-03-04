#pragma once
#include"API.h"
class AI :protected API {
public:
	AI(const int32_t& pID,
		const int32_t& tID,
		std::function<void(const Protobuf::MessageToServer&)> f,
		THUAI4::State*& pS, std::function<void(std::string)>& aM) :\
		API(pID, tID, f, pS, aM) {}
	virtual void play() override;
};



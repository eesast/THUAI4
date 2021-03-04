#pragma once

#ifndef AI_H
#define AI_H

#include "API.h"

class AI : public API
{
public:
	AI(const int32_t& pID,
		const int32_t& tID,
		std::function<void(const Protobuf::MessageToServer&)> f,
		THUAI4::State*& pS, std::function<void(std::string)>& aM) :\
		API(pID, tID, f, pS, aM) {}
	virtual void play() override;
};

#endif	// !AI_H

#pragma once

#ifndef AI_H
#define AI_H

#include "Base.h"

class AI : public AIBase
{
public:
	AI():AIBase() {}
	virtual void play(GameApi&) override;
};

#endif	// !AI_H

#pragma once

#ifndef AI_H
#define AI_H

#include "API.h"

class AI : public API
{
public:

	AI(Logic& l) : API(l) {}

	virtual void play() override;
};

#endif	// !AI_H

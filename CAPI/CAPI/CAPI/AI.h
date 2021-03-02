#pragma once
#include"API.h"
class AI :protected API {
public:
	AI(Logic* l) :API(l) {}
	virtual void play();
};



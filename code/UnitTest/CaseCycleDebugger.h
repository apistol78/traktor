#pragma once

#include "UnitTest/Case.h"

namespace traktor
{

class CaseCycleDebugger : public Case
{
public:
	virtual void run() override final;
};

}


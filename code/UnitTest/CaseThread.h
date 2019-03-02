#pragma once

#include "UnitTest/Case.h"

namespace traktor
{

class CaseThread : public Case
{
public:
	virtual void run() override final;
};

}


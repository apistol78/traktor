#pragma once

#include "UnitTest/Case.h"

namespace traktor
{

class CaseAtomic : public Case
{
public:
	virtual void run() override final;
};

}


#pragma once

#include "UnitTest/Case.h"

namespace traktor
{

class CaseRef : public Case
{
public:
	virtual void run() override final;
};

}

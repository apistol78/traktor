#pragma once

#include "UnitTest/Case.h"

namespace traktor
{

class CaseClone : public Case
{
public:
	virtual void run() override final;
};

}


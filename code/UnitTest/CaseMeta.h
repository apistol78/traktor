#pragma once

#include "UnitTest/Case.h"

namespace traktor
{

class CaseMeta : public Case
{
public:
	virtual void run() override final;
};

}


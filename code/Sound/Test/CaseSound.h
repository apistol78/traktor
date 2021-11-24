#pragma once

#include "Core/Test/Case.h"

namespace traktor
{
	namespace sound
	{
		namespace test
		{

class CaseSound : public traktor::test::Case
{
	T_RTTI_CLASS;

public:
	virtual void run() override final;
};

		}
	}
}

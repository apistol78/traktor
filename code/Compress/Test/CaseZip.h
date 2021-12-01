#pragma once

#include "Core/Test/Case.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_COMPRESS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace compress
	{
		namespace test
		{

class T_DLLCLASS CaseZip : public traktor::test::Case
{
	T_RTTI_CLASS;

public:
	virtual void run() override final;
};

		}
	}
}

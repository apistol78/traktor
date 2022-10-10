#pragma once

#include "Core/Test/Case.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::test
{

class T_DLLCLASS CaseThreadId : public Case
{
	T_RTTI_CLASS;

public:
	virtual void run() override final;
};

}

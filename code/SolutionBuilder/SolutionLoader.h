#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sb
	{

class Solution;

class T_DLLCLASS SolutionLoader : public Object
{
	T_RTTI_CLASS;

public:
	Solution* load(const std::wstring& fileName);

private:
	SmallMap< std::wstring, Ref< Solution > > m_solutions;
};

	}
}


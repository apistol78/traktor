#ifndef SolutionLoader_H
#define SolutionLoader_H

#include <map>
#include <Core/Heap/Ref.h>
#include <Core/Object.h>

class Solution;

class SolutionLoader : public traktor::Object
{
	T_RTTI_CLASS(SolutionLoader)

public:
	Solution* load(const std::wstring& fileName);

private:
	std::map< std::wstring, traktor::Ref< Solution > > m_solutions;
};

#endif	// SolutionLoader_H

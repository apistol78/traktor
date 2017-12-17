/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ExternalDependency_H
#define ExternalDependency_H

#include <Core/RefArray.h>
#include "Dependency.h"

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
class Project;

class T_DLLCLASS ExternalDependency : public Dependency
{
	T_RTTI_CLASS;

public:
	ExternalDependency(const std::wstring& solutionFileName = L"", const std::wstring& projectName = L"");

	void setSolutionFileName(const std::wstring& solutionFileName);

	const std::wstring& getSolutionFileName() const;

	Solution* getSolution() const;

	Project* getProject() const;

	virtual std::wstring getName() const;

	virtual std::wstring getLocation() const;

	virtual bool resolve(SolutionLoader* solutionLoader);

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_solutionFileName;
	std::wstring m_projectName;
	Ref< Solution > m_solution;
	Ref< Project > m_project;
	RefArray< ExternalDependency > m_dependencies;
};

	}
}

#endif	// ExternalDependency_H

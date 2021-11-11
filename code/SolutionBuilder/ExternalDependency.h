#pragma once

#include "Core/RefArray.h"
#include "SolutionBuilder/Dependency.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

	namespace sb
	{

class Solution;
class Project;

class T_DLLCLASS ExternalDependency : public Dependency
{
	T_RTTI_CLASS;

public:
	ExternalDependency() = default;

	explicit ExternalDependency(const std::wstring& solutionFileName, const std::wstring& projectName);

	void setSolutionFileName(const std::wstring& solutionFileName);

	const std::wstring& getSolutionFileName() const;

	Solution* getSolution() const;

	Project* getProject() const;

	virtual std::wstring getName() const override final;

	virtual std::wstring getLocation() const override final;

	virtual bool resolve(const Path& referringSolutionPath, SolutionLoader* solutionLoader) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_solutionFileName;
	std::wstring m_projectName;
	Ref< Solution > m_solution;
	Ref< Project > m_project;
	RefArray< ExternalDependency > m_dependencies;
};

	}
}


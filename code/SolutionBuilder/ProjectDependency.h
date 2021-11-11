#pragma once

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
	namespace sb
	{

class Project;

class T_DLLCLASS ProjectDependency : public Dependency
{
	T_RTTI_CLASS;

public:
	ProjectDependency() = default;

	explicit ProjectDependency(Project* project);

	void setProject(Project* project);

	Project* getProject() const;

	virtual std::wstring getName() const override final;

	virtual std::wstring getLocation() const override final;

	virtual bool resolve(const Path& referringSolutionPath, SolutionLoader* solutionLoader) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< Project > m_project;
};

	}
}


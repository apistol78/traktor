/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef ProjectDependency_H
#define ProjectDependency_H

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
	ProjectDependency(Project* project = 0);

	void setProject(Project* project);

	Project* getProject() const;

	virtual std::wstring getName() const;

	virtual std::wstring getLocation() const;

	virtual bool resolve(const Path& referringSolutionPath, SolutionLoader* solutionLoader);

	virtual void serialize(ISerializer& s);

private:
	Ref< Project > m_project;
};

	}
}

#endif	// ProjectDependency_H

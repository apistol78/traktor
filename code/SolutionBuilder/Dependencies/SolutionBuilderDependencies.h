/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderDependencies_H
#define SolutionBuilderDependencies_H

#include "SolutionBuilder/SolutionBuilder.h"

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

/*! Generate GraphViz visualization of projects and dependencies. */
class T_DLLCLASS SolutionBuilderDependencies : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	virtual bool create(const CommandLine& cmdLine) T_OVERRIDE T_FINAL;

	virtual bool generate(Solution* solution) T_OVERRIDE T_FINAL;

	virtual void showOptions() const T_OVERRIDE T_FINAL;

private:
	std::wstring m_projectName;
};

	}
}

#endif	// SolutionBuilderDependencies_H

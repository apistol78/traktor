/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderGraphViz_H
#define SolutionBuilderGraphViz_H

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
class T_DLLCLASS SolutionBuilderGraphViz : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderGraphViz();

	virtual bool create(const CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	bool m_skipLeafs;
};

	}
}

#endif	// SolutionBuilderGraphViz_H

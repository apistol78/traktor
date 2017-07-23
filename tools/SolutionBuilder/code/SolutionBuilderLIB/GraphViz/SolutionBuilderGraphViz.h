/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderGraphViz_H
#define SolutionBuilderGraphViz_H

#include "SolutionBuilderLIB/SolutionBuilder.h"

/*! Generate GraphViz visualization of projects and dependencies. */
class SolutionBuilderGraphViz : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderGraphViz();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	bool m_skipLeafs;
};

#endif	// SolutionBuilderGraphViz_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvc_H
#define SolutionBuilderMsvc_H

#include "SolutionBuilderLIB/SolutionBuilder.h"

class SolutionBuilderMsvcSettings;

/*! Generate solution files for Visual Studio 2003, 2005, 2008 or 2010. */
class SolutionBuilderMsvc : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvc();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	traktor::Ref< SolutionBuilderMsvcSettings > m_settings;
	bool m_includeExternal;
};

#endif	// SolutionBuilderMsvc_H

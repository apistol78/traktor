/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvc_H
#define SolutionBuilderMsvc_H

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

class SolutionBuilderMsvcSettings;

/*! Generate solution files for Visual Studio 2003, 2005, 2008 or 2010. */
class T_DLLCLASS SolutionBuilderMsvc : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvc();

	virtual bool create(const CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	Ref< SolutionBuilderMsvcSettings > m_settings;
	bool m_includeExternal;
};

	}
}

#endif	// SolutionBuilderMsvc_H

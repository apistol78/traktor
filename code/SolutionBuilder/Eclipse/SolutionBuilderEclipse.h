/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderEclipse_H
#define SolutionBuilderEclipse_H

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

class Project;
class ProjectItem;
class ScriptProcessor;

/*! Generate solution files for Eclipse. */
class T_DLLCLASS SolutionBuilderEclipse : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderEclipse();

	virtual bool create(const CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	Ref< ScriptProcessor > m_scriptProcessor;
	std::wstring m_projectFile;
	std::wstring m_cprojectFile;
};

	}
}

#endif	// SolutionBuilderEclipse_H

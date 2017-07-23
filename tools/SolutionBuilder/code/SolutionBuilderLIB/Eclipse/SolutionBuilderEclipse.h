/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderEclipse_H
#define SolutionBuilderEclipse_H

#include "SolutionBuilderLIB/SolutionBuilder.h"

class Project;
class ProjectItem;
class ScriptProcessor;

/*! Generate solution files for Eclipse. */
class SolutionBuilderEclipse : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderEclipse();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	traktor::Ref< ScriptProcessor > m_scriptProcessor;
	std::wstring m_projectFile;
	std::wstring m_cprojectFile;
};

#endif	// SolutionBuilderEclipse_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderCBlocks_H
#define SolutionBuilderCBlocks_H

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

class ScriptProcessor;

/*! Generate solution files for Code::Blocks. */
class T_DLLCLASS SolutionBuilderCBlocks : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderCBlocks();

	virtual ~SolutionBuilderCBlocks();

	virtual bool create(const CommandLine& cmdLine) T_OVERRIDE T_FINAL;

	virtual bool generate(Solution* solution) T_OVERRIDE T_FINAL;

	virtual void showOptions() const T_OVERRIDE T_FINAL;

private:
	std::wstring m_workspaceTemplate;
	std::wstring m_projectTemplate;
	Ref< ScriptProcessor > m_scriptProcessor;
};

	}
}

#endif	// SolutionBuilderCBlocks_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMake2_H
#define SolutionBuilderMake2_H

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
class T_DLLCLASS SolutionBuilderMake2 : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMake2();

	virtual ~SolutionBuilderMake2();

	virtual bool create(const CommandLine& cmdLine) T_OVERRIDE T_FINAL;

	virtual bool generate(Solution* solution) T_OVERRIDE T_FINAL;

	virtual void showOptions() const T_OVERRIDE T_FINAL;

private:
	std::wstring m_solutionTemplate;
	std::wstring m_projectTemplate;
	Ref< ScriptProcessor > m_scriptProcessor;
};

	}
}

#endif	// SolutionBuilderMake2_H

#pragma once

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

/*! Generate solution files for Xcode. */
class T_DLLCLASS SolutionBuilderXcode2 : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	virtual ~SolutionBuilderXcode2();

	virtual bool create(const CommandLine& cmdLine) override final;

	virtual bool generate(Solution* solution) override final;

	virtual void showOptions() const override final;

private:
	std::wstring m_projectTemplate;
	std::wstring m_workspaceTemplate;
	Ref< ScriptProcessor > m_scriptProcessor;
};

	}
}


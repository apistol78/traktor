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

class SolutionBuilderMsvcSettings;

/*! Generate solution files for Visual Studio 2008, 2010, 2012, 2015 and 2017. */
class T_DLLCLASS SolutionBuilderMsvc : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	virtual bool create(const CommandLine& cmdLine) override final;

	virtual bool generate(Solution* solution) override final;

	virtual void showOptions() const override final;

private:
	Ref< SolutionBuilderMsvcSettings > m_settings;
	bool m_includeExternal = false;
};

	}
}


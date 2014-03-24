#ifndef SolutionBuilderDependencies_H
#define SolutionBuilderDependencies_H

#include "SolutionBuilderLIB/SolutionBuilder.h"

/*! Generate GraphViz visualization of projects and dependencies. */
class SolutionBuilderDependencies : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	std::wstring m_projectName;
};

#endif	// SolutionBuilderDependencies_H

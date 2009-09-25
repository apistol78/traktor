#ifndef SolutionBuilderXcode_H
#define SolutionBuilderXcode_H

#include "SolutionBuilderLIB/SolutionBuilder.h"

class SolutionBuilderXcode : public SolutionBuilder
{
	T_RTTI_CLASS(SolutionBuilderXcode)

public:
	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	bool m_generateStatic;
};

#endif	// SolutionBuilderXcode_H

#ifndef SolutionBuilderMsvc_H
#define SolutionBuilderMsvc_H

#include "SolutionBuilderLIB/SolutionBuilder.h"

class SolutionBuilderMsvcSettings;

/*! Generate solution files for Visual Studio 2003, 2005, 2008 or 2010. */
class SolutionBuilderMsvc : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	traktor::Ref< SolutionBuilderMsvcSettings > m_settings;
};

#endif	// SolutionBuilderMsvc_H

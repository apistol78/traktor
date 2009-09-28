#ifndef SolutionBuilderXcode_H
#define SolutionBuilderXcode_H

#include <Core/Heap/Ref.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"
#include "SolutionBuilderLIB/Configuration.h"

class Project;

class SolutionBuilderXcode : public SolutionBuilder
{
	T_RTTI_CLASS(SolutionBuilderXcode)

public:
	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	std::wstring m_debugConfig;
	std::wstring m_releaseConfig;
	
	void getConfigurations(const Project* project, traktor::RefList< Configuration >& outConfigurations) const;
	
	Configuration::TargetFormat getTargetFormat(const Project* project) const;
};

#endif	// SolutionBuilderXcode_H

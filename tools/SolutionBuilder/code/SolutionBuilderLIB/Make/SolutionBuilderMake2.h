#ifndef SolutionBuilderMake2_H
#define SolutionBuilderMake2_H

#include <Core/Io/Path.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"

class ScriptProcessor;

/*! Generate solution files for Code::Blocks. */
class SolutionBuilderMake2 : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMake2();

	virtual ~SolutionBuilderMake2();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	std::wstring m_solutionTemplate;
	std::wstring m_projectTemplate;
	traktor::Ref< ScriptProcessor > m_scriptProcessor;
};

#endif	// SolutionBuilderMake2_H

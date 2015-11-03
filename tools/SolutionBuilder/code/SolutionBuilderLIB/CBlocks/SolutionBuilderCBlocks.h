#ifndef SolutionBuilderCBlocks_H
#define SolutionBuilderCBlocks_H

#include <Core/Io/Path.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"

class ScriptProcessor;

/*! Generate solution files for Code::Blocks. */
class SolutionBuilderCBlocks : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderCBlocks();

	virtual ~SolutionBuilderCBlocks();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	std::wstring m_workspaceTemplate;
	std::wstring m_projectTemplate;
	traktor::Ref< ScriptProcessor > m_scriptProcessor;
};

#endif	// SolutionBuilderCBlocks_H

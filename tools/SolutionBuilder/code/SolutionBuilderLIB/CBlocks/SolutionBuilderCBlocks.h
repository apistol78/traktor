#ifndef SolutionBuilderCBlocks_H
#define SolutionBuilderCBlocks_H

#include <Core/Io/Path.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"

//class Project;
//class ProjectItem;
//class Solution;
class ScriptProcessor;

/*! Generate solution files for Code::Blocks. */
class SolutionBuilderCBlocks : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderCBlocks();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	traktor::Ref< ScriptProcessor > m_scriptProcessor;
	//std::wstring m_compiler;

	//void collectEmbedItems(const Solution* solution, const Project* project, const ProjectItem* item, std::set< traktor::Path >& outEmbedItems) const;

	//void addItem(const Solution* solution, const Project* project, const ProjectItem* item, traktor::OutputStream& os) const;
};

#endif	// SolutionBuilderCBlocks_H

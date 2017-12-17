/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcLinkerTool_H
#define SolutionBuilderMsvcLinkerTool_H

#include <map>
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcTool.h"

namespace traktor
{
	namespace sb
	{

class ProjectItem;

/*! \brief Visual Studio linker tool. */
class SolutionBuilderMsvcLinkerTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcLinkerTool();

	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, OutputStream& os) const;

	virtual void serialize(ISerializer& s);

private:
	bool m_resolvePaths;
	bool m_resolveFullLibraryPaths;
	std::map< std::wstring, std::wstring > m_staticOptions;

	void findDefinitions(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const RefArray< ProjectItem >& items
	) const;

	void collectAdditionalLibraries(
		Project* project,
		Configuration* configuration,
		bool includeExternal,
		std::set< std::wstring >& outAdditionalLibraries,
		std::set< std::wstring >& outAdditionalLibraryPaths
	) const;
};

	}
}

#endif	// SolutionBuilderMsvcLinkerTool_H

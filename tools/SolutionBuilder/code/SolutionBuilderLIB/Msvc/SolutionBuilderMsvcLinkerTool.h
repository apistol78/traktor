#ifndef SolutionBuilderMsvcLinkerTool_H
#define SolutionBuilderMsvcLinkerTool_H

#include <map>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcTool.h"

class ProjectItem;

/*! \brief Visual Studio linker tool. */
class SolutionBuilderMsvcLinkerTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS(SolutionBuilderMsvcLinkerTool)

public:
	SolutionBuilderMsvcLinkerTool();

	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const;

	virtual bool serialize(traktor::Serializer& s);

private:
	bool m_resolvePaths;
	std::map< std::wstring, std::wstring > m_staticOptions;

	void findDefinitions(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const traktor::RefList< ProjectItem >& items
	) const;

	void collectAdditionalLibraries(
		Project* project,
		Configuration* configuration,
		std::set< std::wstring >& outAdditionalLibraries,
		std::set< std::wstring >& outAdditionalLibraryPaths
	) const;
};

#endif	// SolutionBuilderMsvcLinkerTool_H

#ifndef SolutionBuilderMsvcManifestTool_H
#define SolutionBuilderMsvcManifestTool_H

#include <map>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcTool.h"

class ProjectItem;

/*! \brief Visual Studio manifest tool. */
class SolutionBuilderMsvcManifestTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcManifestTool();

	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const;

	virtual void serialize(traktor::ISerializer& s);

private:
	std::wstring m_manifest;

	void findManifests(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const traktor::RefArray< ProjectItem >& items
	) const;
};

#endif	// SolutionBuilderMsvcManifestTool_H

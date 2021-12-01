#pragma once

#include <map>
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcTool.h"

namespace traktor
{
	namespace sb
	{

class ProjectItem;

/*! Visual Studio manifest tool. */
class SolutionBuilderMsvcManifestTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS;

public:
	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, OutputStream& os) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_manifest;

	void findManifests(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const RefArray< ProjectItem >& items
	) const;
};

	}
}


/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcManifestTool_H
#define SolutionBuilderMsvcManifestTool_H

#include <map>
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcTool.h"

namespace traktor
{
	namespace sb
	{

class ProjectItem;

/*! \brief Visual Studio manifest tool. */
class SolutionBuilderMsvcManifestTool : public SolutionBuilderMsvcTool
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcManifestTool();

	virtual bool generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, OutputStream& os) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

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

#endif	// SolutionBuilderMsvcManifestTool_H

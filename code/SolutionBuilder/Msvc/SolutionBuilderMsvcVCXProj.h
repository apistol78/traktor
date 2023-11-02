/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include <vector>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor::sb
{

class Configuration;
class GeneratorContext;
class Project;
class ProjectItem;
class Solution;
class SolutionBuilderMsvcVCXDefinition;
class SolutionBuilderMsvcVCXBuildTool;
class SolutionBuilderMsvcVCXImportCommon;
class SolutionBuilderMsvcVCXPropertyGroup;

/*! Visual Studio solution project settings. */
class SolutionBuilderMsvcVCXProj : public ISerializable
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXProj();

	std::wstring getPlatform() const;

	bool getInformation(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		std::wstring& outSolutionPath,
		std::wstring& outProjectPath,
		std::wstring& outProjectFileName,
		std::wstring& outProjectGuid
	) const;

	bool generate(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project
	) const;

	virtual void serialize(ISerializer& s) override final;

protected:
	std::wstring m_platform;
	std::wstring m_keyword;
	std::wstring m_windowsTargetPlatformVersion;
	std::wstring m_toolset;
	std::wstring m_targetPrefixes[4];
	std::wstring m_targetExts[4];
	bool m_resolvePaths;
	RefArray< SolutionBuilderMsvcVCXPropertyGroup > m_propertyGroupsBeforeImports;
	RefArray< SolutionBuilderMsvcVCXImportCommon > m_imports;
	RefArray< SolutionBuilderMsvcVCXPropertyGroup > m_propertyGroupsAfterImports;
	std::map< std::wstring, std::wstring > m_configurationDefinitionsDebug[4];
	std::map< std::wstring, std::wstring > m_configurationDefinitionsRelease[4];
	RefArray< SolutionBuilderMsvcVCXDefinition > m_buildDefinitionsDebug[4];
	RefArray< SolutionBuilderMsvcVCXDefinition > m_buildDefinitionsRelease[4];
	RefArray< SolutionBuilderMsvcVCXBuildTool > m_buildTools;

	bool generateProject(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project
	) const;

	bool generateFilters(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project
	) const;

	bool generateUser(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project
	) const;

	bool collectFiles(
		const Project* project,
		const ProjectItem* item,
		const std::wstring& filterPath,
		std::vector< std::pair< std::wstring, Path > >& outFiles
	) const;

	void findDefinitions(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		const RefArray< ProjectItem >& items
	) const;
};

}

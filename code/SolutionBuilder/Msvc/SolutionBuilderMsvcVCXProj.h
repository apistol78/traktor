/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderMsvcVCXProj_H
#define SolutionBuilderMsvcVCXProj_H

#include "SolutionBuilder/Msvc/SolutionBuilderMsvcProject.h"

namespace traktor
{
	namespace sb
	{

class Configuration;
class ProjectItem;
class SolutionBuilderMsvcVCXDefinition;
class SolutionBuilderMsvcVCXBuildTool;
class SolutionBuilderMsvcVCXImportCommon;
class SolutionBuilderMsvcVCXPropertyGroup;

/*! \brief Visual Studio solution project settings. */
class SolutionBuilderMsvcVCXProj : public SolutionBuilderMsvcProject
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXProj();

	virtual std::wstring getPlatform() const;

	virtual bool getInformation(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		std::wstring& outProjectPath,
		std::wstring& outProjectFileName,
		std::wstring& outProjectGuid
	) const;

	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const;

	virtual void serialize(ISerializer& s);

protected:
	std::wstring m_platform;
	std::wstring m_keyword;
	std::wstring m_windowsTargetPlatformVersion;
	std::wstring m_toolset;
	std::wstring m_targetPrefixes[4];
	std::wstring m_targetExts[4];
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
		Solution* solution,
		Project* project
	) const;

	bool generateFilters(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const;

	bool generateUser(
		GeneratorContext& context,
		Solution* solution,
		Project* project
	) const;

	bool collectFiles(
		Project* project,
		ProjectItem* item,
		const std::wstring& filterPath,
		std::vector< std::pair< std::wstring, Path > >& outFiles
	) const;

	void findDefinitions(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const RefArray< ProjectItem >& items
	) const;
};

	}
}

#endif	// SolutionBuilderMsvcVCXProj_H

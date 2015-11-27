#ifndef SolutionBuilderMsvcVCXProj_H
#define SolutionBuilderMsvcVCXProj_H

#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcProject.h"

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

	virtual void serialize(traktor::ISerializer& s);

protected:
	std::wstring m_platform;
	std::wstring m_keyword;
	std::wstring m_toolset;
	std::wstring m_targetPrefixes[4];
	std::wstring m_targetExts[4];
	traktor::RefArray< SolutionBuilderMsvcVCXPropertyGroup > m_propertyGroups;
	traktor::RefArray< SolutionBuilderMsvcVCXImportCommon > m_imports;
	std::map< std::wstring, std::wstring > m_configurationDefinitionsDebug[4];
	std::map< std::wstring, std::wstring > m_configurationDefinitionsRelease[4];
	traktor::RefArray< SolutionBuilderMsvcVCXDefinition > m_buildDefinitionsDebug[4];
	traktor::RefArray< SolutionBuilderMsvcVCXDefinition > m_buildDefinitionsRelease[4];
	traktor::RefArray< SolutionBuilderMsvcVCXBuildTool > m_buildTools;

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
		std::vector< std::pair< std::wstring, traktor::Path > >& outFiles
	) const;

	void findDefinitions(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const traktor::RefArray< ProjectItem >& items
	) const;
};

#endif	// SolutionBuilderMsvcVCXProj_H

#ifndef SolutionBuilderMsvc2005_H
#define SolutionBuilderMsvc2005_H

#include <map>
#include <string>
#include <Core/Serialization/Serializable.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"

class Project;
class ProjectItem;
class Configuration;

namespace traktor
{

class OutputStream;

}

class SolutionBuilderMsvc2005 : public SolutionBuilder
{
	T_RTTI_CLASS(SolutionBuilderMsvc2005)

public:
	struct SettingsTarget
	{
		std::wstring compilerTool;
		std::wstring linkerTool;
		std::wstring definitions;
		std::wstring additionalCompilerOptionsDebug;
		std::wstring additionalCompilerOptionsRelease;
		std::wstring additionalLinkerOptions;
		std::wstring additionalDependencies;
		std::wstring machine;
		int subsystem;
		int characterSet;
		int runtimeLibraryDebug;
		int runtimeLibraryRelease;
		std::wstring additionalManifestFiles;
		std::wstring preBuildEvent;
		std::wstring preLinkEvent;
		std::wstring postBuildEvent;

		SettingsTarget();

		bool serialize(traktor::Serializer& s);
	};

	class Settings : public traktor::Serializable
	{
		T_RTTI_CLASS(Settings)

	public:
		std::wstring platform;
		std::wstring keyword;
		bool resolvePaths;
		SettingsTarget targets[4];

		Settings();

		virtual bool serialize(traktor::Serializer& s);
	};

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	Settings m_settings;
	std::map< Project*, std::wstring > m_projectGuids;

	bool generateProject(Solution* solution, Project* project);

	void addItem(
		traktor::OutputStream& s,
		Solution* solution,
		Project* project,
		ProjectItem* item
	);

	void collectAdditionalLibraries(
		Project* project,
		Configuration* configuration,
		std::set< std::wstring >& additionalLibraries,
		std::set< std::wstring >& additionalLibraryPaths
	);

	std::wstring resolvePath(const std::wstring& path) const;
};

#endif	// SolutionBuilderMsvc2005_H

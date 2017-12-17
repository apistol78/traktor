/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef SolutionBuilderXcode_H
#define SolutionBuilderXcode_H

#include "SolutionBuilder/SolutionBuilder.h"
#include "SolutionBuilder/Configuration.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class OutputStream;
class Path;

	namespace sb
	{

class Aggregation;
class Project;
class Solution;

class T_DLLCLASS SolutionBuilderXcode : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderXcode();

	virtual bool create(const CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	struct ResolvedDependency
	{
		Ref< const Solution > solution;
		Ref< const Project > project;
		bool external;

		bool operator < (const ResolvedDependency& rh) const
		{
			if (solution < rh.solution)
				return true;
			else if (solution > rh.solution)
				return false;
			else if (project < rh.project)
				return true;
			else
				return false;
		}
	};

	std::wstring m_debugConfig;
	std::wstring m_releaseConfig;
	bool m_iphone;
	std::wstring m_projectConfigurationFileDebug;
	std::wstring m_projectConfigurationFileRelease;
	std::wstring m_targetConfigurationFileDebug;
	std::wstring m_targetConfigurationFileRelease;
	std::wstring m_rootSuffix;

	void generatePBXBuildFileSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXBuildRuleSection(OutputStream& s, const Solution* solution) const;

	void generatePBXContainerItemProxySection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXCopyFilesBuildPhaseSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXFileReferenceSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects, const std::set< Path >& files) const;

	void generatePBXFrameworksBuildPhaseSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXGroupSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXAggregateTargetSection(OutputStream& s, const Solution* solution) const;

	void generatePBXNativeTargetSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXProjectSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXReferenceProxySection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXHeadersBuildPhaseSection(OutputStream& s, const RefArray< Project >& projects) const;

	void generatePBXResourcesBuildPhaseSection(OutputStream& s, const RefArray< Project >& projects) const;

	void generatePBXShellScriptBuildPhaseSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generatePBXSourcesBuildPhaseSection(OutputStream& s, const RefArray< Project >& projects) const;

	void generatePBXTargetDependencySection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generateXCBuildConfigurationSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;

	void generateXCConfigurationListSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const;
	
	void getConfigurations(const Project* project, Ref< Configuration > outConfigurations[2]) const;
	
	Configuration::TargetFormat getTargetFormat(const Project* project) const;

	std::wstring getProductType(Configuration::TargetFormat targetFormat) const;
	
	std::wstring getProductName(const Project* project, Configuration::TargetFormat targetFormat) const;

	std::wstring getProductNameNoSuffix(const Project* project, Configuration::TargetFormat targetFormat) const;
	
	void collectCopyDependencies(const Solution* solution, const Project* project, std::set< ResolvedDependency >& outDependencies, bool parentExternal) const;

	void collectLinkDependencies(const Solution* solution, const Project* project, std::set< ResolvedDependency >& outDependencies, bool parentExternal) const;

	void collectCopyDependencies(const Solution* solution, const Aggregation* aggregation, std::set< ResolvedDependency >& outDependencies, bool parentExternal) const;

	bool includeFile(OutputStream& s, const Path& fileName, int32_t indent) const;
};

	}
}

#endif	// SolutionBuilderXcode_H

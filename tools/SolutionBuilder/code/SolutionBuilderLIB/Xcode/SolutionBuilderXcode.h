#ifndef SolutionBuilderXcode_H
#define SolutionBuilderXcode_H

#include <Core/Io/OutputStream.h>
#include <Core/Io/Path.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"
#include "SolutionBuilderLIB/Configuration.h"

class Aggregation;
class Project;
class Solution;

class SolutionBuilderXcode : public SolutionBuilder
{
	T_RTTI_CLASS;

public:
	SolutionBuilderXcode();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	struct ResolvedDependency
	{
		traktor::Ref< const Solution > solution;
		traktor::Ref< const Project > project;
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

	void generatePBXBuildFileSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXBuildRuleSection(traktor::OutputStream& s, const Solution* solution) const;

	void generatePBXContainerItemProxySection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXCopyFilesBuildPhaseSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXFileReferenceSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects, const std::set< traktor::Path >& files) const;

	void generatePBXFrameworksBuildPhaseSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXGroupSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXAggregateTargetSection(traktor::OutputStream& s, const Solution* solution) const;

	void generatePBXNativeTargetSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXProjectSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXReferenceProxySection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXHeadersBuildPhaseSection(traktor::OutputStream& s, const traktor::RefArray< Project >& projects) const;

	void generatePBXResourcesBuildPhaseSection(traktor::OutputStream& s, const traktor::RefArray< Project >& projects) const;

	void generatePBXShellScriptBuildPhaseSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generatePBXSourcesBuildPhaseSection(traktor::OutputStream& s, const traktor::RefArray< Project >& projects) const;

	void generatePBXTargetDependencySection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generateXCBuildConfigurationSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;

	void generateXCConfigurationListSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const;
	
	void getConfigurations(const Project* project, traktor::Ref< Configuration > outConfigurations[2]) const;
	
	Configuration::TargetFormat getTargetFormat(const Project* project) const;

	std::wstring getProductType(Configuration::TargetFormat targetFormat) const;
	
	std::wstring getProductName(const Project* project, Configuration::TargetFormat targetFormat) const;

	std::wstring getProductNameNoSuffix(const Project* project, Configuration::TargetFormat targetFormat) const;
	
	void collectCopyDependencies(const Solution* solution, const Project* project, std::set< ResolvedDependency >& outDependencies, bool parentExternal) const;

	void collectLinkDependencies(const Solution* solution, const Project* project, std::set< ResolvedDependency >& outDependencies, bool parentExternal) const;

	void collectCopyDependencies(const Solution* solution, const Aggregation* aggregation, std::set< ResolvedDependency >& outDependencies, bool parentExternal) const;

	bool includeFile(traktor::OutputStream& s, const traktor::Path& fileName, int32_t indent) const;
};

#endif	// SolutionBuilderXcode_H

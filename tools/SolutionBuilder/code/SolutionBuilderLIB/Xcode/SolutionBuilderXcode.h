#ifndef SolutionBuilderXcode_H
#define SolutionBuilderXcode_H

#include <Core/Heap/Ref.h>
#include <Core/Io/OutputStream.h>
#include <Core/Io/Path.h>
#include "SolutionBuilderLIB/SolutionBuilder.h"
#include "SolutionBuilderLIB/Configuration.h"

class Project;

class SolutionBuilderXcode : public SolutionBuilder
{
	T_RTTI_CLASS(SolutionBuilderXcode)

public:
	SolutionBuilderXcode();

	virtual bool create(const traktor::CommandLine& cmdLine);

	virtual bool generate(Solution* solution);

	virtual void showOptions() const;

private:
	std::wstring m_debugConfig;
	std::wstring m_releaseConfig;
	bool m_iphone;

	void generatePBXBuildFileSection(traktor::OutputStream& s, const traktor::RefList< Project >& projects) const;

	void generatePBXBuildRuleSection(traktor::OutputStream& s, const Solution* solution) const;

	void generatePBXContainerItemProxySection(traktor::OutputStream& s, const Solution* solution, const traktor::RefList< Project >& projects) const;

	void generatePBXFileReferenceSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefList< Project >& projects, const std::set< traktor::Path >& files) const;

	void generatePBXFrameworksBuildPhaseSection(traktor::OutputStream& s, const traktor::RefList< Project >& projects) const;

	void generatePBXGroupSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefList< Project >& projects) const;

	void generatePBXNativeTargetSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefList< Project >& projects) const;

	void generatePBXProjectSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefList< Project >& projects) const;

	void generatePBXReferenceProxySection(traktor::OutputStream& s, const traktor::RefList< Project >& projects) const;

	void generatePBXHeadersBuildPhaseSection(traktor::OutputStream& s, const traktor::RefList< Project >& projects) const;

	void generatePBXSourcesBuildPhaseSection(traktor::OutputStream& s, const traktor::RefList< Project >& projects) const;

	void generatePBXTargetDependencySection(traktor::OutputStream& s, const traktor::RefList< Project >& projects) const;

	void generateXCBuildConfigurationSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefList< Project >& projects) const;

	void generateXCConfigurationListSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefList< Project >& projects) const;
	
	void getConfigurations(const Project* project, traktor::Ref< Configuration > outConfigurations[2]) const;
	
	Configuration::TargetFormat getTargetFormat(const Project* project) const;

	std::wstring getProductType(Configuration::TargetFormat targetFormat) const;
	
	std::wstring getProductName(const Project* project, Configuration::TargetFormat targetFormat) const;
};

#endif	// SolutionBuilderXcode_H

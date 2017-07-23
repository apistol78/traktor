/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Guid.h>
#include <Core/RefSet.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/IStream.h>
#include <Core/Io/Path.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include <Core/Misc/Adler32.h>
#include <Core/Serialization/DeepHash.h>
#include "SolutionBuilderLIB/Aggregation.h"
#include "SolutionBuilderLIB/AggregationItem.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Xcode/SolutionBuilderXcode.h"

using namespace traktor;

namespace
{

	template < typename T1, typename T2, typename T3 >
	struct triplet
	{
		T1 first;
		T2 second;
		T3 third;

		typedef triplet< T1, T2, T3 > type_t;

		triplet< T1, T2, T3 >(T1 t1, T2 t2, T3 t3)
		:	first(t1)
		,	second(t2)
		,	third(t3)
		{
		}

		bool operator < (const type_t& rh) const
		{
			if (third < rh.third)
				return true;
			if (third > rh.third)
				return false;

			if (second < rh.second)
				return true;
			if (second > rh.second)
				return false;

			if (first < rh.first)
				return true;
			if (first > rh.first)
				return false;

			return false;
		}

		static type_t make_triplet(T1 t1, T2 t2, T3 t3)
		{
			return type_t(t1, t2, t3);
		}
	};

	const wchar_t c_hex[] = { L"0123456789ABCDEF" };

	std::set< std::wstring > g_uids;
	std::map< std::pair< const Object*, int32_t >, std::wstring > g_objectUids;
	std::map< std::pair< std::wstring, int32_t >, std::wstring > g_fileUids;
	std::map< triplet< const Object*, std::wstring, int32_t >::type_t, std::wstring > g_objectFileUids;
	std::map< triplet< const Object*, const Object*, int32_t >::type_t, std::wstring > g_objectObjectUids;

	std::wstring createNewUid()
	{
		std::wstring uid;

		for (;;)
		{
			Guid guid = Guid::create();

			StringOutputStream ss;
			for (int i = 0; i < 12; ++i)
			{
				uint8_t p = guid[i];
				ss << c_hex[p >> 4] << c_hex[p & 15];
			}

			uid = ss.str();
			if (g_uids.find(uid) == g_uids.end())
				break;
		}

		g_uids.insert(uid);
		return uid;
	}

	std::wstring calculateUid(const Object* object, int32_t index)
	{
		std::map< std::pair< const Object*, int32_t >, std::wstring >::const_iterator i = g_objectUids.find(std::make_pair(object, index));
		if (i != g_objectUids.end())
			return i->second;

		std::wstring uid = createNewUid();
		g_objectUids[std::make_pair(object, index)] = uid;

		return uid;
	}

	std::wstring calculateUid(const Path& path, int32_t index)
	{
		Path absolutePath = FileSystem::getInstance().getAbsolutePath(path);
		std::wstring absolutePathStr = absolutePath.getPathName();

		std::map< std::pair< std::wstring, int32_t >, std::wstring >::const_iterator i = g_fileUids.find(std::make_pair(absolutePathStr, index));
		if (i != g_fileUids.end())
			return i->second;

		std::wstring uid = createNewUid();
		g_fileUids[std::make_pair(absolutePathStr, index)] = uid;

		return uid;
	}

	std::wstring calculateUid(const Object* object, const Path& path, int32_t index)
	{
		Path absolutePath = FileSystem::getInstance().getAbsolutePath(path);
		std::wstring absolutePathStr = absolutePath.getPathName();

		std::map< triplet< const Object*, std::wstring, int32_t >::type_t, std::wstring >::const_iterator i = g_objectFileUids.find(triplet< const Object*, std::wstring, int32_t >::make_triplet(object, absolutePathStr, index));
		if (i != g_objectFileUids.end())
			return i->second;

		std::wstring uid = createNewUid();
		g_objectFileUids[triplet< const Object*, std::wstring, int32_t >::make_triplet(object, absolutePathStr, index)] = uid;

		return uid;
	}

	std::wstring calculateUid(const Object* object1, const Object* object2, int32_t index)
	{
		std::map< triplet< const Object*, const Object*, int32_t >::type_t, std::wstring >::const_iterator i = g_objectObjectUids.find(triplet< const Object*, const Object*, int32_t >::make_triplet(object1, object2, index));
		if (i != g_objectObjectUids.end())
			return i->second;

		std::wstring uid = createNewUid();
		g_objectObjectUids[triplet< const Object*, const Object*, int32_t >::make_triplet(object1, object2, index)] = uid;

		return uid;
	}

	class FileUids
	{
	public:
		FileUids(const Path& path)
		:	m_path(path)
		{
		}

		std::wstring getFileUid() const { return calculateUid(m_path, -1); }

	private:
		Path m_path;
	};

	class SolutionUids
	{
	public:
		SolutionUids(const Solution* solution)
		:	m_solution(solution)
		{
		}

		std::wstring getProjectUid() const { return calculateUid(m_solution, -1); }

		std::wstring getCustomBuildRuleUid(int32_t index) const { return calculateUid(m_solution, index); }

		std::wstring getBuildConfigurationListUid() const { return calculateUid(m_solution, -3); }

		std::wstring getBuildConfigurationDebugUid() const { return calculateUid(m_solution, -4); }
		
		std::wstring getBuildConfigurationReleaseUid() const { return calculateUid(m_solution, -5); }

		std::wstring getGroupUid() const { return calculateUid(m_solution, -6); }

		std::wstring getProductsGroupUid() const { return calculateUid(m_solution, -7); }

	private:
		Ref< const Solution > m_solution;
	};

	class ProjectUids
	{
	public:
		ProjectUids(const Project* project)
		:	m_project(project)
		{
		}

		std::wstring getGroupUid() const { return calculateUid(m_project, -1); }

		std::wstring getBuildConfigurationListUid() const { return calculateUid(m_project, -2); }

		std::wstring getBuildConfigurationDebugUid() const { return calculateUid(m_project, -3); }

		std::wstring getBuildConfigurationReleaseUid() const { return calculateUid(m_project, -4); }

		std::wstring getBuildPhaseHeadersUid() const { return calculateUid(m_project, -5); }

		std::wstring getBuildPhaseResourcesUid() const { return calculateUid(m_project, -6); }

		std::wstring getBuildPhaseSourcesUid() const { return calculateUid(m_project, -7); }

		std::wstring getBuildPhaseFrameworksUid() const { return calculateUid(m_project, -8); }
		
		std::wstring getBuildPhaseCopyFilesUid() const { return calculateUid(m_project, -9); }

		std::wstring getBuildPhaseShellScriptUid() const { return calculateUid(m_project, -10); }

		std::wstring getTargetUid() const { return calculateUid(m_project, -11); }

		std::wstring getProductUid() const { return calculateUid(m_project, -12); }

		std::wstring getBuildFileUid(const Path& file) const { return calculateUid(m_project, file, -1); }

		std::wstring getBuildFileCopyFileUid(const Path& file) const { return calculateUid(m_project, file, -2); }
		
		std::wstring getBuildFileUid(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -1); }

		std::wstring getBuildFileCopyFileUid(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -2); }
		
		std::wstring getContainerItemProxy(const Project* ownerProject) const { return calculateUid(m_project, ownerProject, -3); }

		std::wstring getContainerItemProxy(const Aggregation* ownerAggregation) const { return calculateUid(m_project, ownerAggregation, -4); }

		std::wstring getTargetDependencyUid(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -5); }

		std::wstring getExternalProductGroupUid(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -6); }

	private:
		Ref< const Project > m_project;
	};

	class AggregationUids
	{
	public:
		AggregationUids(const Aggregation* aggregation)
		:	m_aggregation(aggregation)
		{
		}

		std::wstring getGroupUid() const { return calculateUid(m_aggregation, -1); }

		std::wstring getBuildConfigurationListUid() const { return calculateUid(m_aggregation, -2); }

		std::wstring getBuildConfigurationDebugUid() const { return calculateUid(m_aggregation, -3); }

		std::wstring getBuildConfigurationReleaseUid() const { return calculateUid(m_aggregation, -4); }

		std::wstring getBuildPhaseCopyFilesUid() const { return calculateUid(m_aggregation, -5); }

		std::wstring getBuildPhaseCopyFilesUid(const Path& targetPath) const { return calculateUid(m_aggregation, targetPath, -1); }

		std::wstring getBuildFileCopyFileUid(const Path& file) const { return calculateUid(m_aggregation, file, -2); }

		std::wstring getBuildFileCopyFileUid(const Project* dependencyProject) const { return calculateUid(m_aggregation, dependencyProject, -1); }

		std::wstring getTargetUid() const { return calculateUid(m_aggregation, -6); }

		std::wstring getTargetDependencyUid(const Project* dependencyProject) const { return calculateUid(m_aggregation, dependencyProject, -2); }

	private:
		Ref< const Aggregation > m_aggregation;
	};

	void collectCopyPhases(const RefArray< AggregationItem >& items, std::set< Path >& outPaths)
	{
		for (RefArray< AggregationItem >::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			Path targetPath = Path((*i)->getTargetPath()).getPathName();
			outPaths.insert(targetPath);
		}
	}

	void collectProjectFiles(const Project* project, const RefArray< ProjectItem >& items, std::set< Path >& outFiles)
	{
		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			if (const ::File* file = dynamic_type_cast< const ::File* >(*i))
				file->getSystemFiles(project->getSourcePath(), outFiles);
			else if (const Filter* filter = dynamic_type_cast< const Filter* >(*i))
				collectProjectFiles(project, filter->getItems(), outFiles);
		}
	}

	void collectProjectFiles(const Project* project, std::set< Path >& outFiles)
	{
		const RefArray< ProjectItem >& items = project->getItems();
		collectProjectFiles(project, items, outFiles);
	}

	void collectFrameworks(const Project* project, std::set< std::wstring >& outFrameworks)
	{
		const RefArray< Configuration >& configurations = project->getConfigurations();
		for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
		{
			const std::vector< std::wstring >& libraries = (*i)->getLibraries();
			for (std::vector< std::wstring >::const_iterator j = libraries.begin(); j != libraries.end(); ++j)
			{
				if (endsWith< std::wstring >(*j, L".framework"))
					outFrameworks.insert(*j);
			}
		}

		const RefArray< Dependency >& dependencies = project->getDependencies();
		for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		{
			Ref< const Project > dependencyProject;
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*i))
				dependencyProject = projectDependency->getProject();
			else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*i))
				dependencyProject = externalDependency->getProject();
			if (dependencyProject)
				collectFrameworks(dependencyProject, outFrameworks);
		}
	}

	void createGroups(
		const Project* project,
		OutputStream& s,
		const std::wstring& groupUid,
		const std::wstring& groupName,
		const RefArray< ProjectItem >& items,
		bool addFrameworks
	)
	{
		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			if (const Filter* filter = dynamic_type_cast< const Filter* >(*i))
			{
				std::wstring childGroupUid = calculateUid(filter, -1);
				createGroups(
					project,
					s,
					childGroupUid,
					filter->getName(),
					filter->getItems(),
					false
				);
			}
		}

		s << L"\t\t" << groupUid << L" = /* " << groupName << L" */ {" << Endl;
		s << L"\t\t\tisa = PBXGroup;" << Endl;
		s << L"\t\t\tchildren = (" << Endl;

		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			if (const Filter* filter = dynamic_type_cast< const Filter* >(*i))
			{
				std::wstring childGroupUid = calculateUid(filter, -1);
				s << L"\t\t\t\t" << childGroupUid << L" /* " << filter->getName() << L" */," << Endl;
			}
			else if (const ::File* file = dynamic_type_cast< const ::File* >(*i))
			{
				std::set< Path > files;
				file->getSystemFiles(project->getSourcePath(), files);
				for (std::set< Path >::const_iterator j = files.begin(); j != files.end(); ++j)
				{
					std::wstring childFileUid = FileUids(*j).getFileUid();
					s << L"\t\t\t\t" << childFileUid << L" /* " << j->getFileName() << L" */," << Endl;
				}
			}
		}

		if (addFrameworks)
		{
			std::set< std::wstring > frameworks;
			collectFrameworks(project, frameworks);

			for (std::set< std::wstring >::const_iterator i = frameworks.begin(); i != frameworks.end(); ++i)
			{
				std::wstring childFileUid = FileUids(*i).getFileUid();
				s << L"\t\t\t\t" << childFileUid << L" /* " << *i << L" */," << Endl;
			}
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tname = \"" << groupName << L"\";" << Endl;
		s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
		s << L"\t\t};" << Endl;
	}

}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderXcode", SolutionBuilderXcode, SolutionBuilder)

SolutionBuilderXcode::SolutionBuilderXcode()
:	m_iphone(false)
{
}

bool SolutionBuilderXcode::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption('d', L"xcode-debug-config"))
		m_debugConfig = cmdLine.getOption('d', L"xcode-debug-config").getString();
	else
		m_debugConfig = L"Debug";

	if (cmdLine.hasOption('r', L"xcode-release-config"))
		m_releaseConfig = cmdLine.getOption('r', L"xcode-release-config").getString();
	else
		m_releaseConfig = L"Release";

	if (cmdLine.hasOption('i', L"xcode-configuration"))
	{
		m_iphone = true;
		m_projectConfigurationFileDebug = L"$(TRAKTOR_HOME)/bin/xcode-project-debug-ios.inc";
		m_projectConfigurationFileRelease = L"$(TRAKTOR_HOME)/bin/xcode-project-release-ios.inc";
		m_targetConfigurationFileDebug = L"$(TRAKTOR_HOME)/bin/xcode-target-debug-ios.inc";
		m_targetConfigurationFileRelease = L"$(TRAKTOR_HOME)/bin/xcode-target-release-ios.inc";
	}
	else
	{
		m_iphone = false;
		m_projectConfigurationFileDebug = L"$(TRAKTOR_HOME)/bin/xcode-project-debug-osx.inc";
		m_projectConfigurationFileRelease = L"$(TRAKTOR_HOME)/bin/xcode-project-release-osx.inc";
		m_targetConfigurationFileDebug = L"$(TRAKTOR_HOME)/bin/xcode-target-debug-osx.inc";
		m_targetConfigurationFileRelease = L"$(TRAKTOR_HOME)/bin/xcode-target-release-osx.inc";
	}

	if (cmdLine.hasOption(L"xcode-root-suffix"))
		m_rootSuffix = cmdLine.getOption(L"xcode-root-suffix").getString();
		
	return true;
}

struct DisabledProjectPred
{
	bool operator () (const Project* project) const
	{
		return !project->getEnable();
	}
};

bool SolutionBuilderXcode::generate(Solution* solution)
{
	// Get projects; remove disabled projects.
	RefArray< Project > unsorted = solution->getProjects();	
	for (RefArray< Project >::iterator i = unsorted.begin(); i != unsorted.end(); )
	{
		if (!(*i)->getEnable())
		{
			i = unsorted.erase(i);
			continue;
		}
		++i;
	}

	// Sort projects by their dependencies.
	RefArray< Project > projects;
	while (!unsorted.empty())
	{
		RefArray< Project >::iterator i = unsorted.begin();
		while (i != unsorted.end())
		{
			bool satisfied = true;
			const RefArray< Dependency >& dependencies = (*i)->getDependencies();
			for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!is_a< ProjectDependency >(*j))
					continue;

				Project* dependencyProject = static_cast< ProjectDependency* >(*j)->getProject();
				if (std::find(projects.begin(), projects.end(), dependencyProject) == projects.end())
				{
					satisfied = false;
					break;
				}
			}
			if (satisfied)
				break;
			++i;
		}
		projects.push_back(*i);
		unsorted.erase(i);
	}
	
	// Collect all files references from solution.
	std::set< Path > files;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		collectProjectFiles(*i, files);

	std::wstring solutionRoot = FileSystem::getInstance().getAbsolutePath(solution->getRootPath()).getPathName();
	std::wstring xcodeProjectPath = solutionRoot + m_rootSuffix + L"/" + solution->getName() + L".xcodeproj";

	if (!FileSystem::getInstance().makeAllDirectories(xcodeProjectPath))
		return false;

	Ref< IStream > file = FileSystem::getInstance().open(
		xcodeProjectPath + L"/project.pbxproj",
		traktor::File::FmWrite
	);
	if (!file)
		return false;

	FileOutputStream s(file, new Utf8Encoding(), OutputStream::LeUnix);

	s << L"// !$*UTF8*$!" << Endl;
	s << L"{" << Endl;
	s << L"\tarchiveVersion = 1;" << Endl;
	s << L"\tclasses = {" << Endl;
	s << L"\t};" << Endl;
	s << L"\tobjectVersion = 46;" << Endl;
	s << L"\tobjects = {" << Endl;
	s << Endl;

	generatePBXBuildFileSection(s, solution, projects);
	generatePBXBuildRuleSection(s, solution);
	generatePBXContainerItemProxySection(s, solution, projects);
	generatePBXCopyFilesBuildPhaseSection(s, solution, projects);
	generatePBXFileReferenceSection(s, solution, projects, files);
	generatePBXFrameworksBuildPhaseSection(s, solution, projects);
	generatePBXGroupSection(s, solution, projects);
	generatePBXAggregateTargetSection(s, solution);
	generatePBXNativeTargetSection(s, solution, projects);
	generatePBXProjectSection(s, solution, projects);
	generatePBXReferenceProxySection(s, solution, projects);
	generatePBXHeadersBuildPhaseSection(s, projects);
	generatePBXShellScriptBuildPhaseSection(s, solution, projects);
	generatePBXResourcesBuildPhaseSection(s, projects);
	generatePBXSourcesBuildPhaseSection(s, projects);
	generatePBXTargetDependencySection(s, solution, projects);
	generateXCBuildConfigurationSection(s, solution, projects);
	generateXCConfigurationListSection(s, solution, projects);

	s << L"\t};" << Endl;

	s << L"\trootObject = " << SolutionUids(solution).getProjectUid() << L" /* Project object */;" << Endl;
	s << L"}" << Endl;

	s.close();

	return true;
}

void SolutionBuilderXcode::showOptions() const
{
	log::info << L"\t-d,-xcode-debug-config = Debug configuration" << Endl;
	log::info << L"\t-r,-xcode-release-config = Release configuration" << Endl;
	log::info << L"\t-i,-xcode-configuration = iPhone OS (iphone, ipad)" << Endl;
	log::info << L"\t-xcode-root-suffix = Root path suffix" << Endl;
}

void SolutionBuilderXcode::generatePBXBuildFileSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	s << L"/* Begin PBXBuildFile section */" << Endl;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);

		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring fileUid = FileUids(*j).getFileUid();
			std::wstring buildFileUid = ProjectUids(*i).getBuildFileUid(*j);

			std::wstring extension = toLower(j->getExtension());
			if (extension == L"c" || extension == L"cc" || extension == L"cpp" || extension == L"m" || extension == L"mm" || extension == L"png" || extension == L"xdi")
				s << L"\t\t" << buildFileUid << L" /* " << j->getFileName() << L" in Sources */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << j->getFileName() << L" */; };" << Endl;
			else if (extension == L"h" || extension == L"hh" || extension == L"hpp")
				s << L"\t\t" << buildFileUid << L" /* " << j->getFileName() << L" in Headers */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << j->getFileName() << L" */; };" << Endl;
			else if (extension == L"xib")
				s << L"\t\t" << buildFileUid << L" /* " << j->getFileName() << L" in Resources */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << j->getFileName() << L" */; };" << Endl;
		}
	}

	// Copy files.
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);
		if (
			targetFormat == Configuration::TfSharedLibrary ||
			targetFormat == Configuration::TfExecutable ||
			targetFormat == Configuration::TfExecutableConsole
		)
		{
			std::set< ResolvedDependency > dependencies;
			collectCopyDependencies(solution, *i, dependencies, false);

			for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				Configuration::TargetFormat targetFormat = getTargetFormat(j->project);

				std::wstring productUid = ProjectUids(j->project).getProductUid();
				std::wstring productName = getProductName(j->project, targetFormat);

				s << L"\t\t" << ProjectUids(*i).getBuildFileUid(j->project) << L" /* " << productName << L" in Frameworks */ = { isa = PBXBuildFile; fileRef = " << productUid << L" /* " << productName << L" */; };" << Endl;
			}

			std::set< std::wstring > frameworks;
			collectFrameworks(*i, frameworks);

			for (std::set< std::wstring >::const_iterator j = frameworks.begin(); j != frameworks.end(); ++j)
			{
				std::wstring buildFileUid = ProjectUids(*i).getBuildFileUid(*j);
				std::wstring fileUid = FileUids(*j).getFileUid();
				s << L"\t\t" << buildFileUid << L" /* " << *j << L" in Frameworks */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << *j << L" */; };" << Endl;
			}
		}
		
		if (targetFormat == Configuration::TfExecutable)
		{
			std::set< ResolvedDependency > dependencies;
			collectCopyDependencies(solution, *i, dependencies, false);

			for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
				if (targetFormat != Configuration::TfSharedLibrary)
					continue;
				
				std::wstring productUid = ProjectUids(j->project).getProductUid();
				std::wstring productName = getProductName(j->project, targetFormat);

				s << L"\t\t" << ProjectUids(*i).getBuildFileCopyFileUid(j->project) << L" /* " << productName << L" in CopyFiles */ = { isa = PBXBuildFile; fileRef = " << productUid << L" /* " << productName << L" */; };" << Endl;
			}
		}
	}

	// Copy files into aggregates.
	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
			if (
				targetFormat != Configuration::TfSharedLibrary &&
				targetFormat != Configuration::TfExecutable &&
				targetFormat != Configuration::TfExecutableConsole
			)
				continue;

			std::wstring productUid = ProjectUids(j->project).getProductUid();
			std::wstring productName = getProductName(j->project, targetFormat);

			s << L"\t\t" << AggregationUids(*i).getBuildFileCopyFileUid(j->project) << L" /* " << productName << L" in CopyFiles */ = { isa = PBXBuildFile; fileRef = " << productUid << L" /* " << productName << L" */; };" << Endl;
		}

		const RefArray< AggregationItem >& items = (*i)->getItems();
		for (RefArray< AggregationItem >::const_iterator k = items.begin(); k != items.end(); ++k)
		{
			Path sourceFile((*k)->getSourceFile());
			Path targetPath((*k)->getTargetPath());
			Path targetFile = targetPath.getPathName() + L"/" + sourceFile.getFileName();

			std::wstring fileUid = FileUids(sourceFile).getFileUid();
			s << L"\t\t" << AggregationUids(*i).getBuildFileCopyFileUid(targetFile) << L" /* " << sourceFile.getFileName() << L" in CopyFiles */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << sourceFile.getFileName() << L" */; };" << Endl;
		}
	}

	s << L"/* End PBXBuildFile section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXBuildRuleSection(OutputStream& s, const Solution* solution) const
{
	s << L"/* Begin PBXBuildRule section */" << Endl;
	
	s << L"\t\t" << SolutionUids(solution).getCustomBuildRuleUid(0) << L" /* PBXBuildRule */ = {" << Endl;
	s << L"\t\t\tisa = PBXBuildRule;" << Endl;
	s << L"\t\t\tcompilerSpec = com.apple.compilers.proxy.script;" << Endl;
	s << L"\t\t\tfilePatterns = \"*.png\";" << Endl;
	s << L"\t\t\tfileType = pattern.proxy;" << Endl;
	s << L"\t\t\tisEditable = 1;" << Endl;
	s << L"\t\t\toutputFiles = (" << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tscript = \"\";" << Endl;
	s << L"\t\t};" << Endl;
	
	s << L"\t\t" << SolutionUids(solution).getCustomBuildRuleUid(1) << L" /* PBXBuildRule */ = {" << Endl;
	s << L"\t\t\tisa = PBXBuildRule;" << Endl;
	s << L"\t\t\tcompilerSpec = com.apple.compilers.proxy.script;" << Endl;
	s << L"\t\t\tfilePatterns = \"*.xdi\";" << Endl;
	s << L"\t\t\tfileType = pattern.proxy;" << Endl;
	s << L"\t\t\tisEditable = 1;" << Endl;
	s << L"\t\t\toutputFiles = (" << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tscript = \"\";" << Endl;
	s << L"\t\t};" << Endl;
	
	s << L"/* End PBXBuildRule section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXContainerItemProxySection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	RefSet< const Project > localProjects;
	RefSet< const Project > externalProjects;

	s << L"/* Begin PBXContainerItemProxy section */" << Endl;
	
	// Project's local proxies.
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (j->external || localProjects.find(j->project) != localProjects.end())
				continue;

			s << L"\t\t" << ProjectUids(j->project).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */ = {" << Endl;
			s << L"\t\t\tisa = PBXContainerItemProxy;" << Endl;
			s << L"\t\t\tcontainerPortal = " << SolutionUids(solution).getProjectUid() << L" /* Project object */;" << Endl;
			s << L"\t\t\tproxyType = 1;" << Endl;
			s << L"\t\t\tremoteGlobalIDString = " << ProjectUids(j->project).getTargetUid() << L" /* " << j->project->getName() << L" */;" << Endl;
			s << L"\t\t\tremoteInfo = \"" << j->project->getName() << L"\";" << Endl;
			s << L"\t\t};" << Endl;
			
			localProjects.insert(j->project);
		}
	}

	// Aggregations local proxies.
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (j->external || localProjects.find(j->project) != localProjects.end())
				continue;

			s << L"\t\t" << ProjectUids(j->project).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */ = {" << Endl;
			s << L"\t\t\tisa = PBXContainerItemProxy;" << Endl;
			s << L"\t\t\tcontainerPortal = " << SolutionUids(solution).getProjectUid() << L" /* Project object */;" << Endl;
			s << L"\t\t\tproxyType = 1;" << Endl;
			s << L"\t\t\tremoteGlobalIDString = " << ProjectUids(j->project).getTargetUid() << L" /* " << j->project->getName() << L" */;" << Endl;
			s << L"\t\t\tremoteInfo = \"" << j->project->getName() << L"\";" << Endl;
			s << L"\t\t};" << Endl;

			localProjects.insert(j->project);
		}
	}
	
	// Project's external proxies.
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (!j->external || externalProjects.find(j->project) != externalProjects.end())
				continue;
			
			std::wstring externalSolutionRoot = FileSystem::getInstance().getAbsolutePath(j->solution->getRootPath()).getPathName();
			Path externalXcodeProjectPath = externalSolutionRoot + m_rootSuffix + L"/" + j->solution->getName() + L".xcodeproj";

			Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
			std::wstring externalProductName = getProductName(j->project, targetFormat);

			s << L"\t\t" << ProjectUids(j->project).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */ = {" << Endl;
			s << L"\t\t\tisa = PBXContainerItemProxy;" << Endl;
			s << L"\t\t\tcontainerPortal = " << FileUids(externalXcodeProjectPath).getFileUid() << L" /* " << externalXcodeProjectPath.getFileName() << L" */;" << Endl;
			s << L"\t\t\tproxyType = 2;" << Endl;
			s << L"\t\t\tremoteGlobalIDString = " << createNewUid() << L";" << Endl;
			s << L"\t\t\tremoteInfo = \"" << j->project->getName() << L"\";" << Endl;
			s << L"\t\t};" << Endl;
			
			externalProjects.insert(j->project);
		}
	}

	// Aggregations external proxies.
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (!j->external || externalProjects.find(j->project) != externalProjects.end())
				continue;

			std::wstring externalSolutionRoot = FileSystem::getInstance().getAbsolutePath(j->solution->getRootPath()).getPathName();
			Path externalXcodeProjectPath = externalSolutionRoot + m_rootSuffix + L"/" + j->solution->getName() + L".xcodeproj";

			Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
			std::wstring externalProductName = getProductName(j->project, targetFormat);

			s << L"\t\t" << ProjectUids(j->project).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */ = {" << Endl;
			s << L"\t\t\tisa = PBXContainerItemProxy;" << Endl;
			s << L"\t\t\tcontainerPortal = " << FileUids(externalXcodeProjectPath).getFileUid() << L" /* " << externalXcodeProjectPath.getFileName() << L" */;" << Endl;
			s << L"\t\t\tproxyType = 2;" << Endl;
			s << L"\t\t\tremoteGlobalIDString = " << createNewUid() << L";" << Endl;
			s << L"\t\t\tremoteInfo = \"" << j->project->getName() << L"\";" << Endl;
			s << L"\t\t};" << Endl;

			externalProjects.insert(j->project);
		}
	}
	
	s << L"/* End PBXContainerItemProxy section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXCopyFilesBuildPhaseSection(traktor::OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const
{
	const RefArray< Aggregation >& aggregations = solution->getAggregations();

	// Copy products of dependencies.
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		log::info << L"Copy files phase \"" << (*i)->getName() << L"\"; products..." << Endl;

		s << L"/* Begin PBXCopyFilesBuildPhase section */" << Endl;

		s << L"\t\t" << AggregationUids(*i).getBuildPhaseCopyFilesUid() << L" /* CopyFiles */ = {" << Endl;
		s << L"\t\t\tisa = PBXCopyFilesBuildPhase;" << Endl;
		s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
		s << L"\t\t\tdstPath = \"" << (*i)->getName() << L".app/Contents/MacOS\";" << Endl;
		s << L"\t\t\tdstSubfolderSpec = 16;" << Endl;
		s << L"\t\t\tfiles = (" << Endl;

		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
			if (
				targetFormat != Configuration::TfSharedLibrary &&
				targetFormat != Configuration::TfExecutable &&
				targetFormat != Configuration::TfExecutableConsole
			)
				continue;

			std::wstring productName = getProductName(j->project, targetFormat);
			std::wstring buildFileUid = AggregationUids(*i).getBuildFileCopyFileUid(j->project);
			s << L"\t\t\t\t" << buildFileUid << L" /* " << productName << L" in CopyFiles */," << Endl;

			log::info << L"\tCopy product \"" << productName << L"\"" << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
		s << L"\t\t};" << Endl;

		s << L"/* End PBXCopyFilesBuildPhase section */" << Endl;
		s << Endl;

		const RefArray< AggregationItem >& items = (*i)->getItems();
		if (!items.empty())
		{
			std::set< Path > paths;
			collectCopyPhases(items, paths);
			T_ASSERT (!paths.empty());

			for (std::set< Path >::const_iterator j = paths.begin(); j != paths.end(); ++j)
			{
				log::info << L"Copy files phase \"" << (*i)->getName() << L"\"; into \"" << j->getPathName() << L"\"..." << Endl;

				s << L"/* Begin PBXCopyFilesBuildPhase section */" << Endl;

				s << L"\t\t" << AggregationUids(*i).getBuildPhaseCopyFilesUid(j->getPathName()) << L" /* CopyFiles */ = {" << Endl;
				s << L"\t\t\tisa = PBXCopyFilesBuildPhase;" << Endl;
				s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
				s << L"\t\t\tdstPath = \"" << (*i)->getName() << L".app/" << j->getPathName() << L"\";" << Endl;
				s << L"\t\t\tdstSubfolderSpec = 16;" << Endl;
				s << L"\t\t\tfiles = (" << Endl;

				for (RefArray< AggregationItem >::const_iterator k = items.begin(); k != items.end(); ++k)
				{
					Path sourceFile((*k)->getSourceFile());
					Path targetPath((*k)->getTargetPath());
					Path targetFile = targetPath.getPathName() + L"/" + sourceFile.getFileName();

					if (!(targetPath == *j))
						continue;

					std::wstring buildFileUid = AggregationUids(*i).getBuildFileCopyFileUid(targetFile);
					s << L"\t\t\t\t" << buildFileUid << L" /* " << sourceFile.getFileName() << L" in CopyFiles */," << Endl;

					log::info << L"\tCopy file \"" << sourceFile.getPathName() << L"\" => \"" << targetFile.getPathName() << L"\"" << Endl;
				}

				s << L"\t\t\t);" << Endl;
				s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
				s << L"\t\t};" << Endl;

				s << L"/* End PBXCopyFilesBuildPhase section */" << Endl;
				s << Endl;
			}
		}
	}
}

void SolutionBuilderXcode::generatePBXFileReferenceSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects, const std::set< Path >& files) const
{
	std::set< std::wstring > copiedItems;

	s << L"/* Begin PBXFileReference section */" << Endl;
	for (std::set< Path >::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		std::wstring fileUid = FileUids(*i).getFileUid();

		Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);
		Path filePath = FileSystem::getInstance().getAbsolutePath(*i);

		Path relativeFilePath;
		if (!FileSystem::getInstance().getRelativePath(filePath, projectPath, relativeFilePath))
			relativeFilePath = filePath;

		s << L"\t\t" << fileUid << L" /* " << i->getFileName() << L" */ = { isa = PBXFileReference; name = " << i->getFileName() << L"; path = " << relativeFilePath.getPathName() << L"; sourceTree = \"<group>\"; };" << Endl;
	}

	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		const RefArray< AggregationItem >& items = (*i)->getItems();
		for (RefArray< AggregationItem >::const_iterator j = items.begin(); j != items.end(); ++j)
		{
			Path sourceFile((*j)->getSourceFile());

			Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);
			Path filePath = FileSystem::getInstance().getAbsolutePath(sourceFile);

			Path relativeFilePath;
			if (!FileSystem::getInstance().getRelativePath(filePath, projectPath, relativeFilePath))
				relativeFilePath = filePath;

			std::wstring fileUid = FileUids(sourceFile).getFileUid();
			if (copiedItems.find(fileUid) == copiedItems.end())
			{
				s << L"\t\t" << fileUid << L" /* " << sourceFile.getFileName() << L" */ = { isa = PBXFileReference; name = " << sourceFile.getFileName() << L"; path = " << relativeFilePath.getPathName() << L"; sourceTree = \"<group>\"; };" << Endl;
				copiedItems.insert(fileUid);
			}
		}
	}

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		std::wstring productUid = ProjectUids(*i).getProductUid();
		std::wstring productName = getProductName(*i, targetFormat);

		s << L"\t\t" << productUid << L" /* " << productName << L" */ = { isa = PBXFileReference; includeInIndex = 0; path = " << productName << L"; sourceTree = BUILT_PRODUCTS_DIR; };" << Endl;
	}

	std::set< Path > externalSolutionPaths;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				if (externalSolutionPaths.find(externalDependency->getSolutionFileName()) != externalSolutionPaths.end())
					continue;

				Ref< const Solution > externalSolution = externalDependency->getSolution();
				T_ASSERT (externalSolution);

				Path externalXcodeProjectPath = FileSystem::getInstance().getAbsolutePath(externalSolution->getRootPath() + m_rootSuffix + L"/" + externalSolution->getName() + L".xcodeproj");
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);

				Path relativeFilePath;
				if (!FileSystem::getInstance().getRelativePath(externalXcodeProjectPath, projectPath, relativeFilePath))
					relativeFilePath = externalXcodeProjectPath;

				s << L"\t\t" << FileUids(externalXcodeProjectPath).getFileUid() << L" /* " << externalXcodeProjectPath.getFileName() << L" */ = { isa = PBXFileReference; lastKnownFileType = \"wrapper.pb-project\"; name = \"" << relativeFilePath.getFileName() << L"\"; path = \"" << relativeFilePath.getPathName() << L"\"; sourceTree = SOURCE_ROOT; };" << Endl;

				externalSolutionPaths.insert(externalDependency->getSolutionFileName());
			}
		}
	}

	std::set< std::wstring > frameworks;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		collectFrameworks(*i, frameworks);
		
	for (std::set< std::wstring >::const_iterator i = frameworks.begin(); i != frameworks.end(); ++i)
	{
		std::wstring fileUid = FileUids(*i).getFileUid();
		if (!m_iphone)
			s << L"\t\t" << fileUid << L" /* " << *i << L" in Frameworks */ = { isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = " << *i << L"; path = /System/Library/Frameworks/" << *i << L"; sourceTree = \"<absolute>\"; };" << Endl;
		else
			s << L"\t\t" << fileUid << L" /* " << *i << L" in Frameworks */ = { isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = " << *i << L"; path = System/Library/Frameworks/" << *i << L"; sourceTree = SDKROOT; };" << Endl;
	}
	
	s << L"/* End PBXFileReference section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXFrameworksBuildPhaseSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	s << L"/* Begin PBXFrameworksBuildPhase section */" << Endl;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		s << L"\t\t" << ProjectUids(*i).getBuildPhaseFrameworksUid() << L" /* Frameworks */ = {" << Endl;
		s << L"\t\t\tisa = PBXFrameworksBuildPhase;" << Endl;
		s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
		s << L"\t\t\tfiles = (" << Endl;

		Configuration::TargetFormat targetFormat = getTargetFormat(*i);
		if (
			targetFormat == Configuration::TfSharedLibrary ||
			targetFormat == Configuration::TfExecutable ||
			targetFormat == Configuration::TfExecutableConsole
		)
		{
			// Add system frameworks.
			std::set< std::wstring > frameworks;
			collectFrameworks(*i, frameworks);
			
			for (std::set< std::wstring >::const_iterator j = frameworks.begin(); j != frameworks.end(); ++j)
			{
				std::wstring buildFileUid = ProjectUids(*i).getBuildFileUid(*j);
				s << L"\t\t\t\t" << buildFileUid << L" /* " << *j << L" in Frameworks */," << Endl;
			}

			// Add dependent libraries.
			std::set< ResolvedDependency > dependencies;
			collectLinkDependencies(solution, *i, dependencies, false);
			
			log::info << (*i)->getName() << L"..." << Endl;
		
			// Add local dependencies first.
			for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (j->external)
					continue;
				
				Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
				std::wstring productName = getProductName(j->project, targetFormat);
				
				log::info << L"\t+ " << productName << Endl;

				s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(j->project) << L" /* " << productName << L" in Frameworks */," << Endl;
			}

			// Then add external dependencies.
			for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!j->external)
					continue;

				Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
				std::wstring productName = getProductName(j->project, targetFormat);

				log::info << L"\t* " << productName << Endl;

				s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(j->project) << L" /* " << productName << L" in Frameworks */," << Endl;
			}
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXFrameworksBuildPhase section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXGroupSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	RefSet< const Solution > externalSolutions;

	s << L"/* Begin PBXGroup section */" << Endl;
	s << L"\t\t" << SolutionUids(solution).getGroupUid() << L" = {" << Endl;
	s << L"\t\t\tisa = PBXGroup;" << Endl;
	s << L"\t\t\tchildren = (" << Endl;

	// Add external solutions groups.
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (!j->external)
				continue;

			if (externalSolutions.find(j->solution) != externalSolutions.end())
				continue;

			Path externalXcodeProjectPath = FileSystem::getInstance().getAbsolutePath(j->solution->getRootPath() + m_rootSuffix + L"/" + j->solution->getName() + L".xcodeproj");
			Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);

			Path relativeFilePath;
			if (!FileSystem::getInstance().getRelativePath(externalXcodeProjectPath, projectPath, relativeFilePath))
				relativeFilePath = externalXcodeProjectPath;

			s << L"\t\t\t\t" << FileUids(externalXcodeProjectPath).getFileUid() << L" /* " << externalXcodeProjectPath.getFileName() << L" */," << Endl;

			externalSolutions.insert(j->solution);
		}
	}

	// Add projects groups.
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::wstring projectGroupUid = ProjectUids(*i).getGroupUid();
		s << L"\t\t\t\t" << projectGroupUid << L" /* " << (*i)->getName() << L" */," << Endl;
	}

	// Add aggregations groups.
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		std::wstring aggregationGroupUid = AggregationUids(*i).getGroupUid();
		s << L"\t\t\t\t" << aggregationGroupUid << L" /* " << (*i)->getName() << L" */," << Endl;
	}

	// Add products group.
	s << L"\t\t\t\t" << SolutionUids(solution).getProductsGroupUid() << L" /* Products */," << Endl;

	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
	s << L"\t\t};" << Endl;

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		createGroups(
			*i,
			s,
			ProjectUids(*i).getGroupUid(),
			(*i)->getName(),
			(*i)->getItems(),
			true
		);
	}

	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		std::set< std::wstring > copiedFiles;

		s << L"\t\t" << AggregationUids(*i).getGroupUid() << L" = /* " << (*i)->getName() << L" */ {" << Endl;
		s << L"\t\t\tisa = PBXGroup;" << Endl;
		s << L"\t\t\tchildren = (" << Endl;

		const RefArray< AggregationItem >& items = (*i)->getItems();
		for (RefArray< AggregationItem >::const_iterator j = items.begin(); j != items.end(); ++j)
		{
			Path sourceFile((*j)->getSourceFile());
			std::wstring fileUid = FileUids(sourceFile).getFileUid();
			if (copiedFiles.find(fileUid) == copiedFiles.end())
			{
				s << L"\t\t\t\t" << fileUid << L" /* " << sourceFile.getFileName() << L" */," << Endl;
				copiedFiles.insert(fileUid);
			}
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tname = \"" << (*i)->getName() << L"\";" << Endl;
		s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
		s << L"\t\t};" << Endl;
	}

	s << L"\t\t" << SolutionUids(solution).getProductsGroupUid() << L" = /* Products */ {" << Endl;
	s << L"\t\t\tisa = PBXGroup;" << Endl;
	s << L"\t\t\tchildren = (" << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tname = Products;" << Endl;
	s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
	s << L"\t\t};" << Endl;

	for (RefSet< const Solution >::const_iterator i = externalSolutions.begin(); i != externalSolutions.end(); ++i)
	{
		s << L"\t\t" << SolutionUids(*i).getProductsGroupUid() << L" = /* Products */ {" << Endl;
		s << L"\t\t\tisa = PBXGroup;" << Endl;
		s << L"\t\t\tchildren = (" << Endl;

		const RefArray< Project >& externalProjects = (*i)->getProjects();
		for (RefArray< Project >::const_iterator i = externalProjects.begin(); i != externalProjects.end(); ++i)
		{
			Configuration::TargetFormat targetFormat = getTargetFormat(*i);

			std::wstring productUid = ProjectUids(*i).getProductUid();
			std::wstring productName = getProductName(*i, targetFormat);

			s << L"\t\t\t\t" << productUid << L" /* " << productName << L" */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tname = Products;" << Endl;
		s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
		s << L"\t\t};" << Endl;
	}

	s << L"/* End PBXGroup section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXAggregateTargetSection(OutputStream& s, const Solution* solution) const
{
	const RefArray< Aggregation >& aggregations = solution->getAggregations();

	s << L"/* Begin PBXAggregateTarget section */" << Endl;
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		s << L"\t\t" << AggregationUids(*i).getTargetUid() << L" /* " << (*i)->getName() << L" */ = {" << Endl;
		s << L"\t\t\tisa = PBXAggregateTarget;" << Endl;
		s << L"\t\t\tbuildConfigurationList = " << AggregationUids(*i).getBuildConfigurationListUid() << L" /* Build configuration list for PBXAggregateTarget \"" << (*i)->getName() << L"\" */;" << Endl;
		s << L"\t\t\tbuildPhases = (" << Endl;
		s << L"\t\t\t\t" << AggregationUids(*i).getBuildPhaseCopyFilesUid() << L" /* CopyFiles */," << Endl;

		const RefArray< AggregationItem >& items = (*i)->getItems();
		if (!items.empty())
		{
			std::set< Path > paths;
			collectCopyPhases(items, paths);
			T_ASSERT (!paths.empty());

			for (std::set< Path >::const_iterator j = paths.begin(); j != paths.end(); ++j)
				s << L"\t\t\t\t" << AggregationUids(*i).getBuildPhaseCopyFilesUid(*j) << L" /* CopyFiles */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tdependencies = (" << Endl;

		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
				s << L"\t\t\t\t" << AggregationUids(*i).getTargetDependencyUid(projectDependency->getProject()) << L" /* PBXTargetDependency */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tname = \"" << (*i)->getName() << L"\";" << Endl;
		s << L"\t\t\tproductName = \"" << (*i)->getName() << L"\";" << Endl;		
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXAggregateTarget section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXNativeTargetSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	s << L"/* Begin PBXNativeTarget section */" << Endl;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);

		bool needScriptPhase = false;
		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring extension = toLower(j->getExtension());
			if (extension == L"png" || extension == L"xdi")
			{
				needScriptPhase = true;
				break;
			}
		}

		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		s << L"\t\t" << ProjectUids(*i).getTargetUid() << L" /* " << (*i)->getName() << L" */ = {" << Endl;
		s << L"\t\t\tisa = PBXNativeTarget;" << Endl;
		s << L"\t\t\tbuildConfigurationList = " << ProjectUids(*i).getBuildConfigurationListUid() << L" /* Build configuration list for PBXNativeTarget \"" << (*i)->getName() << L"\" */;" << Endl;
		s << L"\t\t\tbuildPhases = (" << Endl;
		if (needScriptPhase)
			s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseShellScriptUid() << L" /* ShellScript */," << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseHeadersUid() << L" /* Headers */," << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseSourcesUid() << L" /* Sources */," << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseResourcesUid() << L" /* Resources */," << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseFrameworksUid() << L" /* Frameworks */," << Endl;
		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tbuildRules = (" << Endl;
		s << L"\t\t\t\t" << SolutionUids(solution).getCustomBuildRuleUid(0) << L" /* PBXBuildRule */," << Endl;
		s << L"\t\t\t\t" << SolutionUids(solution).getCustomBuildRuleUid(1) << L" /* PBXBuildRule */," << Endl;
		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tdependencies = (" << Endl;

		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
				s << L"\t\t\t\t" << ProjectUids(*i).getTargetDependencyUid(projectDependency->getProject()) << L" /* PBXTargetDependency */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tname = \"" << (*i)->getName() << L"\";" << Endl;
		s << L"\t\t\tproductName = \"" << (*i)->getName() << L"\";" << Endl;
		s << L"\t\t\tproductReference = " << ProjectUids(*i).getProductUid() << L" /* " << getProductName(*i, targetFormat) << L" */;" << Endl;
		s << L"\t\t\tproductType = \"" << getProductType(targetFormat) << L"\";" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXNativeTarget section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXProjectSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	std::set< std::wstring > externalSolutionPaths;

	s << L"/* Begin PBXProject section */" << Endl;
	s << L"\t\t" << SolutionUids(solution).getProjectUid() << L" /* Project object */ = {" << Endl;
	s << L"\t\t\tisa = PBXProject;" << Endl;
	s << L"\t\t\tattributes = {" << Endl;
	s << L"\t\t\t\tORGANIZATIONNAME = \"Doctor Entertainment AB\";" << Endl;
	s << L"\t\t\t};" << Endl;
	s << L"\t\t\tbuildConfigurationList = " << SolutionUids(solution).getBuildConfigurationListUid() << L" /* Build configuration list for PBXProject \"" << solution->getName() << L"\" */;" << Endl;
	s << L"\t\t\tcompatibilityVersion = \"Xcode 3.2\";" << Endl;
	s << L"\t\t\tdevelopmentRegion = English;" << Endl;
	s << L"\t\t\thasScannedForEncodings = 0;" << Endl;
	s << L"\t\t\tknownRegions = (" << Endl;
	s << L"\t\t\t\ten," << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tmainGroup = " << SolutionUids(solution).getGroupUid() << L";" << Endl;
	s << L"\t\t\tproductRefGroup = " << SolutionUids(solution).getProductsGroupUid() << L" /* Products */;" << Endl;
	s << L"\t\t\tprojectDirPath = \"\";" << Endl;
	s << L"\t\t\tprojectReferences = (" << Endl;

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				if (externalSolutionPaths.find(externalDependency->getSolutionFileName()) != externalSolutionPaths.end())
					continue;

				Ref< const Solution > externalSolution = externalDependency->getSolution();
				T_ASSERT (externalSolution);

				Path externalXcodeProjectPath = externalSolution->getRootPath() + m_rootSuffix + L"/" + externalSolution->getName() + L".xcodeproj";

				s << L"\t\t\t\t{" << Endl;
				s << L"\t\t\t\t\tProductGroup = " << SolutionUids(externalDependency->getSolution()).getProductsGroupUid() << L" /* Products */;" << Endl;
				s << L"\t\t\t\t\tProjectRef = " << FileUids(externalXcodeProjectPath).getFileUid() << L" /* " << externalXcodeProjectPath.getFileName() << L" */;" << Endl;
				s << L"\t\t\t\t}," << Endl;

				externalSolutionPaths.insert(externalDependency->getSolutionFileName());
			}
		}
	}
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tprojectRoot = \"\";" << Endl;
	s << L"\t\t\ttargets = (" << Endl;

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		s << L"\t\t\t\t" << ProjectUids(*i).getTargetUid() << L" /* " << (*i)->getName() << L" */," << Endl;
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;
		s << L"\t\t\t\t" << AggregationUids(*i).getTargetUid() << L" /* " << (*i)->getName() << L" */," << Endl;
	}

	s << L"\t\t\t);" << Endl;
	s << L"\t\t};" << Endl;
	s << L"/* End PBXProject section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXReferenceProxySection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	RefSet< const Project > externalProjects;

	s << L"/* Begin PBXReferenceProxy section */" << Endl;
	
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (!j->external || externalProjects.find(j->project) != externalProjects.end())
				continue;
				
			Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
			std::wstring productName = getProductName(j->project, targetFormat);
			std::wstring productType = getProductType(targetFormat);

			s << L"\t\t" << ProjectUids(j->project).getProductUid() << L" /* " << productName << L" */ = {" << Endl;
			s << L"\t\t\tisa = PBXReferenceProxy;" << Endl;
			s << L"\t\t\tfileType = \"" << productType << L"\";" << Endl;
			s << L"\t\t\tpath = \"" << productName << L"\";" << Endl;
			s << L"\t\t\tremoteRef = " << ProjectUids(j->project).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */;" << Endl;
			s << L"\t\t\tsourceTree = BUILT_PRODUCTS_DIR;" << Endl;
			s << L"\t\t};" << Endl;
			
			externalProjects.insert(j->project);
		}
	}

	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		std::set< ResolvedDependency > dependencies;
		collectCopyDependencies(solution, *i, dependencies, false);

		for (std::set< ResolvedDependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (!j->external || externalProjects.find(j->project) != externalProjects.end())
				continue;

			Configuration::TargetFormat targetFormat = getTargetFormat(j->project);
			std::wstring productName = getProductName(j->project, targetFormat);
			std::wstring productType = getProductType(targetFormat);

			s << L"\t\t" << ProjectUids(j->project).getProductUid() << L" /* " << productName << L" */ = {" << Endl;
			s << L"\t\t\tisa = PBXReferenceProxy;" << Endl;
			s << L"\t\t\tfileType = \"" << productType << L"\";" << Endl;
			s << L"\t\t\tpath = \"" << productName << L"\";" << Endl;
			s << L"\t\t\tremoteRef = " << ProjectUids(j->project).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */;" << Endl;
			s << L"\t\t\tsourceTree = BUILT_PRODUCTS_DIR;" << Endl;
			s << L"\t\t};" << Endl;

			externalProjects.insert(j->project);
		}
	}
	
	s << L"/* End PBXReferenceProxy section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXHeadersBuildPhaseSection(OutputStream& s, const RefArray< Project >& projects) const
{
	s << L"/* Begin PBXHeadersBuildPhase section */" << Endl;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		s << L"\t\t" << ProjectUids(*i).getBuildPhaseHeadersUid() << L" /* Headers */ = {" << Endl;
		s << L"\t\t\tisa = PBXHeadersBuildPhase;" << Endl;
		s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
		s << L"\t\t\tfiles = (" << Endl;

		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);
		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring extension = toLower(j->getExtension());
			if (extension == L"h" || extension == L"hh" || extension == L"hpp")
				s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(*j) << L" /* " << j->getFileName() << L" */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXHeadersBuildPhase section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXResourcesBuildPhaseSection(traktor::OutputStream& s, const traktor::RefArray< Project >& projects) const
{
	s << L"/* Begin PBXResourcesBuildPhase section */" << Endl;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		s << L"\t\t" << ProjectUids(*i).getBuildPhaseResourcesUid() << L" /* Resources */ = {" << Endl;
		s << L"\t\t\tisa = PBXResourcesBuildPhase;" << Endl;
		s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
		s << L"\t\t\tfiles = (" << Endl;

		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);
		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring extension = toLower(j->getExtension());
			if (extension == L"xib")
				s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(*j) << L" /* " << j->getFileName() << L" in Resources */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXResourcesBuildPhase section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXShellScriptBuildPhaseSection(OutputStream& s, const Solution* solution, const traktor::RefArray< Project >& projects) const
{
	Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);

	s << L"/* Begin PBXShellScriptBuildPhase section */" << Endl;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);

		std::set< Path > resourceFiles;
		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring extension = toLower(j->getExtension());
			if (extension == L"png" || extension == L"xdi")
			{
				Path filePath = FileSystem::getInstance().getAbsolutePath(*j);

				Path relativeFilePath;
				if (FileSystem::getInstance().getRelativePath(filePath, projectPath, relativeFilePath))
					resourceFiles.insert(relativeFilePath);
				else
					log::warning << L"Unable to determine relative path of \"" << filePath.getPathName() << L"\"; not added to project" << Endl;
			}
		}

		if (resourceFiles.empty())
			continue;

		s << L"\t" << ProjectUids(*i).getBuildPhaseShellScriptUid() << L" /* ShellScript */ = {" << Endl;
		s << L"\t\tisa = PBXShellScriptBuildPhase;" << Endl;
		s << L"\t\tbuildActionMask = 2147483647;" << Endl;
		s << L"\t\tfiles = (" << Endl;
		s << L"\t\t);" << Endl;
		s << L"\t\tinputPaths = (" << Endl;

		for (std::set< Path >::const_iterator j = resourceFiles.begin(); j != resourceFiles.end(); ++j)
			s << L"\t\t\t\"" << j->getPathName() << L"\"," << Endl;

		s << L"\t\t);" << Endl;
		s << L"\t\toutputPaths = (" << Endl;

		for (std::set< Path >::const_iterator j = resourceFiles.begin(); j != resourceFiles.end(); ++j)
			s << L"\t\t\t\"$(DERIVED_FILE_DIR)/Resources/" << j->getFileNameNoExtension() << L".h\"," << Endl;
		
		s << L"\t\t);" << Endl;
		s << L"\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
		s << L"\t\tshellPath = /bin/sh;" << Endl;

		s << L"\t\tshellScript = \"";
		s << L"# DO NOT EDIT!\\n";

		for (std::set< Path >::const_iterator j = resourceFiles.begin(); j != resourceFiles.end(); ++j)
			s << L"BinaryInclude \\\"" << j->getPathName() << L"\\\" \\\"${DERIVED_FILE_DIR}/Resources/" << j->getFileNameNoExtension() << L".h\\\" c_Resource" << j->getFileNameNoExtension() << L"\\n";

		s << L"\";" << Endl;
		s << L"\t};" << Endl;
	}
	s << L"/* End PBXShellScriptBuildPhase section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXSourcesBuildPhaseSection(OutputStream& s, const RefArray< Project >& projects) const
{
	s << L"/* Begin PBXSourcesBuildPhase section */" << Endl;
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		s << L"\t\t" << ProjectUids(*i).getBuildPhaseSourcesUid() << L" /* Sources */ = {" << Endl;
		s << L"\t\t\tisa = PBXSourcesBuildPhase;" << Endl;
		s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
		s << L"\t\t\tfiles = (" << Endl;

		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);
		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring extension = toLower(j->getExtension());
			if (extension == L"png" || extension == L"xdi")
				s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(*j) << L" /* " << j->getFileName() << L" */," << Endl;
		}
		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring extension = toLower(j->getExtension());
			if (extension == L"c" || extension == L"cc" || extension == L"cpp" || extension == L"m" || extension == L"mm")
				s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(*j) << L" /* " << j->getFileName() << L" */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXSourcesBuildPhase section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generatePBXTargetDependencySection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	const RefArray< Aggregation >& aggregations = solution->getAggregations();

	s << L"/* Begin PBXTargetDependency section */" << Endl;

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			{
				s << L"\t\t" << ProjectUids(*i).getTargetDependencyUid(projectDependency->getProject()) << L" /* PBXTargetDependency */ = {" << Endl;
				s << L"\t\t\tisa = PBXTargetDependency;" << Endl;
				s << L"\t\t\ttarget = " << ProjectUids(projectDependency->getProject()).getTargetUid() << L" /* " << projectDependency->getProject()->getName() << L" */;" << Endl;
				s << L"\t\t\ttargetProxy = " << ProjectUids(projectDependency->getProject()).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */;" << Endl;
				s << L"\t\t};" << Endl;
			}
		}
	}

	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			{
				s << L"\t\t" << AggregationUids(*i).getTargetDependencyUid(projectDependency->getProject()) << L" /* PBXTargetDependency */ = {" << Endl;
				s << L"\t\t\tisa = PBXTargetDependency;" << Endl;
				s << L"\t\t\ttarget = " << ProjectUids(projectDependency->getProject()).getTargetUid() << L" /* " << projectDependency->getProject()->getName() << L" */;" << Endl;
				s << L"\t\t\ttargetProxy = " << ProjectUids(projectDependency->getProject()).getContainerItemProxy(*i) << L" /* PBXContainerItemProxy */;" << Endl;
				s << L"\t\t};" << Endl;
			}
		}
	}

	s << L"/* End PBXTargetDependency section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generateXCBuildConfigurationSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	s << L"/* Begin XCBuildConfiguration section */" << Endl;

	// Global settings.
	s << L"\t\t" << SolutionUids(solution).getBuildConfigurationDebugUid() << L" /* Debug */ = {" << Endl;
	s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
	s << L"\t\t\tbuildSettings = {" << Endl;

	includeFile(s, m_projectConfigurationFileDebug, 4);

	s << L"\t\t\t};" << Endl;
	s << L"\t\t\tname = Debug;" << Endl;
	s << L"\t\t};" << Endl;

	s << L"\t\t" << SolutionUids(solution).getBuildConfigurationReleaseUid() << L" /* Release */ = {" << Endl;
	s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
	s << L"\t\t\tbuildSettings = {" << Endl;

	includeFile(s, m_projectConfigurationFileRelease, 4);

	s << L"\t\t\t};" << Endl;
	s << L"\t\t\tname = Release;" << Endl;
	s << L"\t\t};" << Endl;

	// Project settings.
	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		if (!(*i)->getEnable())
			continue;

		s << L"\t\t" << AggregationUids(*i).getBuildConfigurationDebugUid() << L" /* " << (*i)->getName() << L" Debug */ = {" << Endl;
		s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
		s << L"\t\t\tbuildSettings = {" << Endl;
		s << L"\t\t\t\tPRODUCT_NAME = \"" << (*i)->getName() << L"\";" << Endl;
		s << L"\t\t\t};" << Endl;
		s << L"\t\t\tname = Debug;" << Endl;
		s << L"\t\t};" << Endl;

		s << L"\t\t" << AggregationUids(*i).getBuildConfigurationReleaseUid() << L" /* " << (*i)->getName() << L" Release */ = {" << Endl;
		s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
		s << L"\t\t\tbuildSettings = {" << Endl;
		s << L"\t\t\t\tPRODUCT_NAME = \"" << (*i)->getName() << L"\";" << Endl;
		s << L"\t\t\t};" << Endl;
		s << L"\t\t\tname = Release;" << Endl;
		s << L"\t\t};" << Endl;
	}

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Configuration > configurations[2];
		getConfigurations(*i, configurations);
		
		// Get project plist file.
		std::wstring plistFile;
		
		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);

		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring extension = toLower(j->getExtension());
			if (extension == L"plist")
			{
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);
				Path plistPath = FileSystem::getInstance().getAbsolutePath(*j);

				Path relativePlistPath;
				if (!FileSystem::getInstance().getRelativePath(plistPath, projectPath, relativePlistPath))
					relativePlistPath = plistPath;

				plistFile = relativePlistPath.getPathName();
				break;
			}
		}

		if (!plistFile.empty())
			traktor::log::info << L"Using plist file \"" << plistFile << L"\"" << Endl;

		// Debug configuration
		if (configurations[0])
		{
			s << L"\t\t" << ProjectUids(*i).getBuildConfigurationDebugUid() << L" /* " << (*i)->getName() << L" Debug */ = {" << Endl;
			s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
			s << L"\t\t\tbuildSettings = {" << Endl;

			includeFile(s, m_targetConfigurationFileDebug, 4);

			s << L"\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"";
			std::vector< std::wstring > definitions = configurations[0]->getDefinitions();
			definitions.insert(definitions.end(), solution->getDefinitions().begin(), solution->getDefinitions().end());
			for (std::vector< std::wstring >::const_iterator j = definitions.begin(); j != definitions.end(); ++j)
			{
				if (j != definitions.begin())
					s << L" ";
				s << *j;
			}
			s << L"\";" << Endl;

			s << L"\t\t\t\tUSER_HEADER_SEARCH_PATHS = \"";
			const std::vector< std::wstring >& includePaths = configurations[0]->getIncludePaths();
			for (std::vector< std::wstring >::const_iterator j = includePaths.begin(); j != includePaths.end(); ++j)
			{
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);
				Path includePath = FileSystem::getInstance().getAbsolutePath(*j);

				Path relativeIncludePath;
				if (!FileSystem::getInstance().getRelativePath(includePath, projectPath, relativeIncludePath))
					relativeIncludePath = includePath;

				s << relativeIncludePath.getPathName() << L" ";
			}
			s << L"'${DERIVED_FILES_DIR}'\";" << Endl;

			s << L"\t\t\t\tPRODUCT_NAME = \"" << getProductNameNoSuffix(*i, configurations[0]->getTargetFormat()) << L"\";" << Endl;
			if (!plistFile.empty())
				s << L"\t\t\t\tINFOPLIST_FILE = \"" << plistFile << L"\";" << Endl;
			
			if (configurations[0]->getTargetFormat() == Configuration::TfSharedLibrary)
				s << L"\t\t\t\tINSTALL_PATH = \"@executable_path\";" << Endl;

			if (
				configurations[0]->getTargetFormat() == Configuration::TfSharedLibrary ||
				configurations[0]->getTargetFormat() == Configuration::TfExecutable ||
				configurations[0]->getTargetFormat() == Configuration::TfExecutableConsole
			)
			{
				const std::vector< std::wstring >& libraryPaths = configurations[0]->getLibraryPaths();
				if (!libraryPaths.empty())
				{
					s << L"\t\t\t\tLIBRARY_SEARCH_PATHS = \"";
					for (std::vector< std::wstring >::const_iterator j = libraryPaths.begin(); j != libraryPaths.end(); ++j)
					{
						Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);
						Path libraryPath = FileSystem::getInstance().getAbsolutePath(*j);

						Path relativeLibraryPath;
						if (!FileSystem::getInstance().getRelativePath(libraryPath, projectPath, relativeLibraryPath))
							relativeLibraryPath = libraryPath;

						s << relativeLibraryPath.getPathName() << L" ";
					}
					s << L"$(inherited) \\\"$(SRCROOT)/build/Debug\\\"\";" << Endl;
				}

				{
					s << L"\t\t\t\tOTHER_LDFLAGS = (" << Endl;

					// Add explicit libraries.
					const std::vector< std::wstring >& libraries = configurations[0]->getLibraries();
					for (std::vector< std::wstring >::const_iterator j = libraries.begin(); j != libraries.end(); ++j)
					{
						if (endsWith< std::wstring >(*j, L".framework"))
							continue;

						s << L"\t\t\t\t\t-l" << *j << L"," << Endl;
					}

					// Add "force load" of all dependencies.
					const RefArray< Dependency >& dependencies = (*i)->getDependencies();
					for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
					{
						if ((*j)->getLink() != Dependency::LnkForce)
							continue;

						std::wstring productName;
						
						if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
						{
							Configuration::TargetFormat targetFormat = getTargetFormat(projectDependency->getProject());
							productName = getProductName(projectDependency->getProject(), targetFormat);
						}
						else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
						{
							Configuration::TargetFormat targetFormat = getTargetFormat(externalDependency->getProject());
							productName = getProductName(externalDependency->getProject(), targetFormat);
						}
						else
							continue;

						s << L"\t\t\t\t\t\"-force_load\"," << Endl;
						s << L"\t\t\t\t\t$BUILT_PRODUCTS_DIR/" << productName << L"," << Endl;
					}

					s << L"\t\t\t\t);" << Endl;
				}
			}

			s << L"\t\t\t};" << Endl;
			s << L"\t\t\tname = Debug;" << Endl;
			s << L"\t\t};" << Endl;
		}

		// Release configuration
		if (configurations[1])
		{
			s << L"\t\t" << ProjectUids(*i).getBuildConfigurationReleaseUid() << L" /* " << (*i)->getName() << L" Release */ = {" << Endl;
			s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
			s << L"\t\t\tbuildSettings = {" << Endl;

			includeFile(s, m_targetConfigurationFileRelease, 4);

			s << L"\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"";
			const std::vector< std::wstring >& definitions = configurations[1]->getDefinitions();
			for (std::vector< std::wstring >::const_iterator j = definitions.begin(); j != definitions.end(); ++j)
			{
				if (j != definitions.begin())
					s << L" ";
				s << *j;
			}
			s << L"\";" << Endl;

			s << L"\t\t\t\tUSER_HEADER_SEARCH_PATHS = \"";
			const std::vector< std::wstring >& includePaths = configurations[1]->getIncludePaths();
			for (std::vector< std::wstring >::const_iterator j = includePaths.begin(); j != includePaths.end(); ++j)
			{
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);
				Path includePath = FileSystem::getInstance().getAbsolutePath(*j);

				Path relativeIncludePath;
				if (!FileSystem::getInstance().getRelativePath(includePath, projectPath, relativeIncludePath))
					relativeIncludePath = includePath;

				s << relativeIncludePath.getPathName() << L" ";
			}
			s << L"'${DERIVED_FILES_DIR}'\";" << Endl;
			
			s << L"\t\t\t\tPRODUCT_NAME = \"" << getProductNameNoSuffix(*i, configurations[1]->getTargetFormat()) << L"\";" << Endl;
			if (!plistFile.empty())
				s << L"\t\t\t\tINFOPLIST_FILE = \"" << plistFile << L"\";" << Endl;

			if (configurations[1]->getTargetFormat() == Configuration::TfSharedLibrary)
				s << L"\t\t\t\tINSTALL_PATH = \"@executable_path\";" << Endl;

			if (
				configurations[1]->getTargetFormat() == Configuration::TfSharedLibrary ||
				configurations[1]->getTargetFormat() == Configuration::TfExecutable ||
				configurations[1]->getTargetFormat() == Configuration::TfExecutableConsole
			)
			{
				const std::vector< std::wstring >& libraryPaths = configurations[1]->getLibraryPaths();
				if (!libraryPaths.empty())
				{
					s << L"\t\t\t\tLIBRARY_SEARCH_PATHS = \"";
					for (std::vector< std::wstring >::const_iterator j = libraryPaths.begin(); j != libraryPaths.end(); ++j)
					{
						Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath() + m_rootSuffix);
						Path libraryPath = FileSystem::getInstance().getAbsolutePath(*j);

						Path relativeLibraryPath;
						if (!FileSystem::getInstance().getRelativePath(libraryPath, projectPath, relativeLibraryPath))
							relativeLibraryPath = libraryPath;

						s << relativeLibraryPath.getPathName() << L" ";
					}
					s << L"$(inherited) \\\"$(SRCROOT)/build/Release\\\"\";" << Endl;
				}

				{
					s << L"\t\t\t\tOTHER_LDFLAGS = (" << Endl;

					// Add explicit libraries.
					const std::vector< std::wstring >& libraries = configurations[1]->getLibraries();
					for (std::vector< std::wstring >::const_iterator j = libraries.begin(); j != libraries.end(); ++j)
					{
						if (endsWith< std::wstring >(*j, L".framework"))
							continue;

						s << L"\t\t\t\t\t-l" << *j << L"," << Endl;
					}

					// Add "force load" of all dependencies.
					const RefArray< Dependency >& dependencies = (*i)->getDependencies();
					for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
					{
						if ((*j)->getLink() != Dependency::LnkForce)
							continue;

						std::wstring productName;

						if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
						{
							Configuration::TargetFormat targetFormat = getTargetFormat(projectDependency->getProject());
							productName = getProductName(projectDependency->getProject(), targetFormat);
						}
						else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
						{
							Configuration::TargetFormat targetFormat = getTargetFormat(externalDependency->getProject());
							productName = getProductName(externalDependency->getProject(), targetFormat);
						}
						else
							continue;

						s << L"\t\t\t\t\t\"-force_load\"," << Endl;
						s << L"\t\t\t\t\t$BUILT_PRODUCTS_DIR/" << productName << L"," << Endl;
					}

					s << L"\t\t\t\t);" << Endl;
				}
			}

			s << L"\t\t\t};" << Endl;
			s << L"\t\t\tname = Release;" << Endl;
			s << L"\t\t};" << Endl;
		}
	}
	s << L"/* End XCBuildConfiguration section */" << Endl;
	s << Endl;
}

void SolutionBuilderXcode::generateXCConfigurationListSection(OutputStream& s, const Solution* solution, const RefArray< Project >& projects) const
{
	s << L"/* Begin XCConfigurationList section */" << Endl;
	s << L"\t\t" << SolutionUids(solution).getBuildConfigurationListUid() << L" /* Build configuration list for PBXProject \"" << solution->getName() << L"\" */ = {" << Endl;
	s << L"\t\t\tisa = XCConfigurationList;" << Endl;
	s << L"\t\t\tbuildConfigurations = (" << Endl;
	s << L"\t\t\t\t" << SolutionUids(solution).getBuildConfigurationDebugUid() << L" /* Debug */," << Endl;
	s << L"\t\t\t\t" << SolutionUids(solution).getBuildConfigurationReleaseUid() << L" /* Release */," << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tdefaultConfigurationIsVisible = 0;" << Endl;
	s << L"\t\t\tdefaultConfigurationName = Debug;" << Endl;
	s << L"\t\t};" << Endl;

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		s << L"\t\t" << ProjectUids(*i).getBuildConfigurationListUid() << L" /* Build configuration list for PBXNativeTarget \"" << (*i)->getName() << L"\" */ = {" << Endl;
		s << L"\t\t\tisa = XCConfigurationList;" << Endl;
		s << L"\t\t\tbuildConfigurations = (" << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildConfigurationDebugUid() << L" /* Debug */," << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildConfigurationReleaseUid() << L" /* Release */," << Endl;
		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tdefaultConfigurationIsVisible = 0;" << Endl;
		s << L"\t\t\tdefaultConfigurationName = Debug;" << Endl;
		s << L"\t\t};" << Endl;
	}

	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		s << L"\t\t" << AggregationUids(*i).getBuildConfigurationListUid() << L" /* Build configuration list for PBXAggregateTarget \"" << (*i)->getName() << L"\" */ = {" << Endl;
		s << L"\t\t\tisa = XCConfigurationList;" << Endl;
		s << L"\t\t\tbuildConfigurations = (" << Endl;
		s << L"\t\t\t\t" << AggregationUids(*i).getBuildConfigurationDebugUid() << L" /* Debug */," << Endl;
		s << L"\t\t\t\t" << AggregationUids(*i).getBuildConfigurationReleaseUid() << L" /* Release */," << Endl;
		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tdefaultConfigurationIsVisible = 0;" << Endl;
		s << L"\t\t\tdefaultConfigurationName = Debug;" << Endl;
		s << L"\t\t};" << Endl;
	}

	s << L"/* End XCConfigurationList section */" << Endl;
}

void SolutionBuilderXcode::getConfigurations(const Project* project, Ref< Configuration > outConfigurations[2]) const
{
	outConfigurations[0] = project->getConfiguration(m_debugConfig);
	outConfigurations[1] = project->getConfiguration(m_releaseConfig);

	if (!outConfigurations[0])
		log::warning << L"Project \"" << project->getName() << L"\" doesn't have debug configuration" << Endl;
		
	if (!outConfigurations[1])
		log::warning << L"Project \"" << project->getName() << L"\" doesn't have release configuration" << Endl;
}

Configuration::TargetFormat SolutionBuilderXcode::getTargetFormat(const Project* project) const
{
	Ref< Configuration > configurations[2];
	getConfigurations(project, configurations);
	
	Configuration::TargetFormat targetFormat;
	
	if (configurations[0])
		targetFormat = configurations[0]->getTargetFormat();
	else if (configurations[1])
		targetFormat = configurations[1]->getTargetFormat();
	else
		T_FATAL_ERROR;
	
	return targetFormat;
}

std::wstring SolutionBuilderXcode::getProductType(Configuration::TargetFormat targetFormat) const
{
	switch (targetFormat)
	{
	case Configuration::TfStaticLibrary:
		return L"com.apple.product-type.library.static";
	case Configuration::TfSharedLibrary:
		return L"com.apple.product-type.library.dynamic";
	case Configuration::TfExecutable:
	case Configuration::TfExecutableConsole:
		if (!m_iphone)
			return L"com.apple.product-type.tool";
		else
			return L"com.apple.product-type.application";
	}
	return L"";
}

std::wstring SolutionBuilderXcode::getProductName(const Project* project, Configuration::TargetFormat targetFormat) const
{
	switch (targetFormat)
	{
	case Configuration::TfStaticLibrary:
		return L"lib" + project->getName() + L".a";
	case Configuration::TfSharedLibrary:
		return L"lib" + project->getName() + L".dylib";
	case Configuration::TfExecutable:
		return project->getName();
	case Configuration::TfExecutableConsole:
		return project->getName();
	}
	return L"";
}

std::wstring SolutionBuilderXcode::getProductNameNoSuffix(const Project* project, Configuration::TargetFormat targetFormat) const
{
	switch (targetFormat)
	{
	case Configuration::TfStaticLibrary:
		return project->getName();
	case Configuration::TfSharedLibrary:
		return L"lib" + project->getName();
	case Configuration::TfExecutable:
		return project->getName();
	case Configuration::TfExecutableConsole:
		return project->getName();
	}
	return L"";
}

void SolutionBuilderXcode::collectCopyDependencies(
	const Solution* solution,
	const Project* project,
	std::set< ResolvedDependency >& outDependencies,
	bool parentExternal
) const
{
	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		ResolvedDependency dependency;

		if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*i))
		{
			dependency.solution = solution;
			dependency.project = projectDependency->getProject();
			dependency.external = parentExternal;
		}
		else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*i))
		{
			dependency.solution = externalDependency->getSolution();
			dependency.project = externalDependency->getProject();
			dependency.external = true;
		}
		else
			continue;

		if (outDependencies.find(dependency) == outDependencies.end())
		{
			outDependencies.insert(dependency);
			collectCopyDependencies(dependency.solution, dependency.project, outDependencies, dependency.external);
		}
	}
}

void SolutionBuilderXcode::collectLinkDependencies(
	const Solution* solution,
	const Project* project,
	std::set< ResolvedDependency >& outDependencies,
	bool parentExternal
) const
{
	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if ((*i)->getLink() == Dependency::LnkNo)
			continue;

		ResolvedDependency dependency;

		if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*i))
		{
			dependency.solution = solution;
			dependency.project = projectDependency->getProject();
			dependency.external = parentExternal;
		}
		else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*i))
		{
			dependency.solution = externalDependency->getSolution();
			dependency.project = externalDependency->getProject();
			dependency.external = true;
		}
		else
			continue;

		if (outDependencies.find(dependency) == outDependencies.end())
		{
			outDependencies.insert(dependency);
			if (getTargetFormat(dependency.project) == Configuration::TfStaticLibrary)
				collectLinkDependencies(dependency.solution, dependency.project, outDependencies, dependency.external);
		}
	}
}

void SolutionBuilderXcode::collectCopyDependencies(
	const Solution* solution,
	const Aggregation* aggregation,
	std::set< ResolvedDependency >& outDependencies,
	bool parentExternal
) const
{
	const RefArray< Dependency >& dependencies = aggregation->getDependencies();
	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		ResolvedDependency dependency;

		if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*i))
		{
			dependency.solution = solution;
			dependency.project = projectDependency->getProject();
			dependency.external = parentExternal;
		}
		else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*i))
		{
			dependency.solution = externalDependency->getSolution();
			dependency.project = externalDependency->getProject();
			dependency.external = true;
		}
		else
			continue;

		if (outDependencies.find(dependency) == outDependencies.end())
		{
			outDependencies.insert(dependency);
			collectCopyDependencies(dependency.solution, dependency.project, outDependencies, dependency.external);
		}
	}
}

bool SolutionBuilderXcode::includeFile(traktor::OutputStream& s, const traktor::Path& fileName, int32_t indent) const
{
	ScopeIndent scopeIndent(s);
	s.setIndent(s.getIndent() + indent);

	Ref< IStream > stream = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!stream)
		return false;

	StringReader sr(stream, new Utf8Encoding());
	std::wstring line;

	while (sr.readLine(line) >= 0)
	{
		line = trim(line);
		if (!line.empty())
			s << line << Endl;
	}

	stream->close();
	return true;
}

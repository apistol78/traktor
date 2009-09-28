#include <Core/Io/FileSystem.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/Stream.h>
#include <Core/Io/Path.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Serialization/DeepHash.h>
#include <Core/Misc/Adler32.h>
#include <Core/Guid.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Xcode/SolutionBuilderXcode.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Filter.h"

using namespace traktor;

namespace
{

	template < typename T1, typename T2, typename T3 >
	struct triplet
	{
		typedef std::pair< T1, std::pair< T2, T3 > > type_t;

		static type_t make_triplet(T1 t1, T2 t2, T3 t3)
		{
			return std::make_pair(t1, std::make_pair(t2, t3));
		}
	};

	const wchar_t c_hex[] = { L"0123456789ABCDEF" };

	std::set< std::wstring > g_uids;
	std::map< std::pair< const Object*, int32_t >, std::wstring > g_objectUids;
	std::map< std::pair< std::wstring, int32_t >, std::wstring > g_fileUids;
	std::map< std::pair< const Object*, std::wstring >, std::wstring > g_objectFileUids;
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

	std::wstring calculateUid(const Object* object, const Path& path)
	{
		Path absolutePath = FileSystem::getInstance().getAbsolutePath(path);
		std::wstring absolutePathStr = absolutePath.getPathName();

		std::map< std::pair< const Object*, std::wstring >, std::wstring >::const_iterator i = g_objectFileUids.find(std::make_pair(object, absolutePathStr));
		if (i != g_objectFileUids.end())
			return i->second;

		std::wstring uid = createNewUid();
		g_objectFileUids[std::make_pair(object, absolutePathStr)] = uid;

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

		std::wstring getBuildConfigurationListUid() const { return calculateUid(m_solution, -2); }

		std::wstring getBuildConfigurationDebugUid() const { return calculateUid(m_solution, -3); }
		
		std::wstring getBuildConfigurationReleaseUid() const { return calculateUid(m_solution, -4); }

		std::wstring getGroupUid() const { return calculateUid(m_solution, -5); }

		std::wstring getProductsGroupUid() const { return calculateUid(m_solution, -6); }

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

		std::wstring getBuildPhaseSourcesUid() const { return calculateUid(m_project, -6); }

		std::wstring getBuildPhaseFrameworksUid() const {return calculateUid(m_project, -7); }

		std::wstring getBuildFileUid(const Path& file) const { return calculateUid(m_project, file); }

		std::wstring getBuildFileUid(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -1); }

		std::wstring getTargetUid() const { return calculateUid(m_project, -8); }

		std::wstring getProductUid() const { return calculateUid(m_project, -9); }

		std::wstring getContainerItemProxy(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -2); }

		std::wstring getTargetDependencyUid(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -3); }

		std::wstring getExternalProductGroupUid(const Project* dependencyProject) const { return calculateUid(m_project, dependencyProject, -4); }

		std::wstring getProductName(Configuration::TargetFormat targetFormat) const
		{
			switch (targetFormat)
			{
			case Configuration::TfStaticLibrary:
				return L"lib" + m_project->getName() + L".a";
			case Configuration::TfSharedLibrary:
				return L"lib" + m_project->getName() + L".dylib";
			case Configuration::TfExecutable:
				return m_project->getName();
			case Configuration::TfExecutableConsole:
				return m_project->getName();
			}
			return L"";
		}

		std::wstring getProductType(Configuration::TargetFormat targetFormat) const
		{
			switch (targetFormat)
			{
			case Configuration::TfStaticLibrary:
				return L"com.apple.product-type.library.static";
			case Configuration::TfSharedLibrary:
				return L"com.apple.product-type.library.dynamic";
			case Configuration::TfExecutable:
				return L"com.apple.product-type.application";
			case Configuration::TfExecutableConsole:
				return L"com.apple.product-type.tool";
			}
			return L"";
		}

	private:
		Ref< const Project > m_project;
	};

	void collectProjectFiles(const Project* project, const RefList< ProjectItem >& items, std::set< Path >& outFiles)
	{
		for (RefList< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			if (const ::File* file = dynamic_type_cast< const ::File* >(*i))
				file->getSystemFiles(project->getSourcePath(), outFiles);
			else if (const Filter* filter = dynamic_type_cast< const Filter* >(*i))
				collectProjectFiles(project, filter->getItems(), outFiles);
		}
	}

	void collectProjectFiles(const Project* project, std::set< Path >& outFiles)
	{
		const RefList< ProjectItem >& items = project->getItems();
		collectProjectFiles(project, items, outFiles);
	}

	void createGroups(
		const Project* project,
		OutputStream& s,
		const std::wstring& groupUid,
		const std::wstring& groupName,
		const RefList< ProjectItem >& items
	)
	{
		for (RefList< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			if (const Filter* filter = dynamic_type_cast< const Filter* >(*i))
			{
				std::wstring childGroupUid = calculateUid(filter, -1);
				createGroups(
					project,
					s,
					childGroupUid,
					filter->getName(),
					filter->getItems()
				);
			}
		}

		s << L"\t\t" << groupUid << L" = /* " << groupName << L" */ {" << Endl;
		s << L"\t\t\tisa = PBXGroup;" << Endl;
		s << L"\t\t\tchildren = (" << Endl;

		for (RefList< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
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

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tname = " << groupName << L";" << Endl;
		s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
		s << L"\t\t};" << Endl;
	}

}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderXcode", SolutionBuilderXcode, SolutionBuilder)

bool SolutionBuilderXcode::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption('d'))
		m_debugConfig = cmdLine.getOption('d').getString();
	else
		m_debugConfig = L"Debug";

	if (cmdLine.hasOption('r'))
		m_releaseConfig = cmdLine.getOption('r').getString();
	else
		m_releaseConfig = L"Release";
		
	return true;
}

bool SolutionBuilderXcode::generate(Solution* solution)
{
	const RefList< Project >& projects = solution->getProjects();

	// Collect all files references from solution.
	std::set< Path > files;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		collectProjectFiles(*i, files);

	std::wstring xcodeProjectPath = solution->getRootPath() + L"/" + solution->getName() + L".xcodeproj";

	if (!FileSystem::getInstance().makeAllDirectories(xcodeProjectPath))
		return false;

	Ref< Stream > file = FileSystem::getInstance().open(
		xcodeProjectPath + L"/project.pbxproj",
		traktor::File::FmWrite
	);
	if (!file)
		return false;

	FileOutputStream s(file, gc_new< Utf8Encoding >(), OutputStream::LeUnix);

	s << L"// !$*UTF8*$!" << Endl;
	s << L"{" << Endl;
	s << L"\tarchiveVersion = 1;" << Endl;
	s << L"\tclasses = {" << Endl;
	s << L"\t};" << Endl;
	s << L"\tobjectVersion = 45;" << Endl;
	s << L"\tobjects = {" << Endl;

	s << Endl;

	s << L"/* Begin PBXBuildFile section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::set< Path > projectFiles;
		collectProjectFiles(*i, projectFiles);

		for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
		{
			std::wstring fileUid = FileUids(*j).getFileUid();
			std::wstring buildFileUid = ProjectUids(*i).getBuildFileUid(*j);

			std::wstring extension = toLower(j->getExtension());
			if (extension == L"c" || extension == L"cc" || extension == L"cpp" || extension == L"m" || extension == L"mm")
				s << L"\t\t" << buildFileUid << L" /* " << j->getFileName() << L" in Sources */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << j->getFileName() << L" */; };" << Endl;
			else if (extension == L"h" || extension == L"hh" || extension == L"hpp")
				s << L"\t\t" << buildFileUid << L" /* " << j->getFileName() << L" in Headers */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << j->getFileName() << L" */; };" << Endl;
		}
	}
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			{
				Configuration::TargetFormat targetFormat = getTargetFormat(projectDependency->getProject());

				std::wstring productUid = ProjectUids(projectDependency->getProject()).getProductUid();
				std::wstring productName = ProjectUids(projectDependency->getProject()).getProductName(targetFormat);

				s << L"\t\t" << ProjectUids(*i).getBuildFileUid(projectDependency->getProject()) << L" /* " << productName << L" in Frameworks */ = { isa = PBXBuildFile; fileRef = " << productUid << L" /* " << productName << L" */; };" << Endl;
			}
			else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				Configuration::TargetFormat targetFormat = getTargetFormat(externalDependency->getProject());

				std::wstring productUid = ProjectUids(*i).getTargetDependencyUid(externalDependency->getProject());
				std::wstring productName = ProjectUids(externalDependency->getProject()).getProductName(targetFormat);

				s << L"\t\t" << ProjectUids(*i).getBuildFileUid(externalDependency->getProject()) << L" /* " << productName << L" in Frameworks */ = { isa = PBXBuildFile; fileRef = " << productUid << L" /* " << productName << L" */; };" << Endl;
			}
		}
	}
	s << L"/* End PBXBuildFile section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXContainerItemProxy section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			{
				s << L"\t\t" << ProjectUids(*i).getContainerItemProxy(projectDependency->getProject()) << L" /* PBXContainerItemProxy */ = {" << Endl;
				s << L"\t\t\tisa = PBXContainerItemProxy;" << Endl;
				s << L"\t\t\tcontainerPortal = " << SolutionUids(solution).getProjectUid() << L" /* Project object */;" << Endl;
				s << L"\t\t\tproxyType = 1;" << Endl;
				s << L"\t\t\tremoteGlobalIDString = " << ProjectUids(projectDependency->getProject()).getTargetUid() << L" /* " << projectDependency->getProject()->getName() << L" */;" << Endl;
				s << L"\t\t\tremoteInfo = " << projectDependency->getProject()->getName() << L";" << Endl;
				s << L"\t\t};" << Endl;
			}
			else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				Ref< const Solution > externalSolution = externalDependency->getSolution();
				T_ASSERT (externalSolution);

				Path externalXcodeProjectPath = externalSolution->getRootPath() + L"/" + externalSolution->getName() + L".xcodeproj";

				s << L"\t\t" << ProjectUids(*i).getContainerItemProxy(externalDependency->getProject()) << L" /* PBXContainerItemProxy */ = {" << Endl;
				s << L"\t\t\tisa = PBXContainerItemProxy;" << Endl;
				s << L"\t\t\tcontainerPortal = " << FileUids(externalXcodeProjectPath).getFileUid() << L" /* " << externalXcodeProjectPath.getFileName() << L" */;" << Endl;
				s << L"\t\t\tproxyType = 2;" << Endl;
				s << L"\t\t\tremoteGlobalIDString = " << ProjectUids(externalDependency->getProject()).getTargetUid() << L" /* " << externalDependency->getProject()->getName() << L" */;" << Endl;
				s << L"\t\t\tremoteInfo = " << externalDependency->getProject()->getName() << L";" << Endl;
				s << L"\t\t};" << Endl;
			}
		}
	}
	s << L"/* End PBXContainerItemProxy section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXFileReference section */" << Endl;
	for (std::set< Path >::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		std::wstring fileUid = FileUids(*i).getFileUid();

		Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());
		Path filePath = FileSystem::getInstance().getAbsolutePath(*i);

		Path relativeFilePath;
		if (!FileSystem::getInstance().getRelativePath(filePath, projectPath, relativeFilePath))
			relativeFilePath = filePath;

		s << L"\t\t" << fileUid << L" /* " << i->getFileName() << L" */ = { isa = PBXFileReference; name = " << i->getFileName() << L"; path = " << relativeFilePath.getPathName() << L"; sourceTree = \"<group>\"; };" << Endl;
	}

	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		std::wstring productUid = ProjectUids(*i).getProductUid();
		std::wstring productName = ProjectUids(*i).getProductName(targetFormat);

		s << L"\t\t" << productUid << L" /* " << productName << L" */ = { isa = PBXFileReference; includeInIndex = 0; path = " << productName << L"; sourceTree = BUILT_PRODUCTS_DIR; };" << Endl;
	}

	std::set< Path > externalSolutionPaths;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				if (externalSolutionPaths.find(externalDependency->getSolutionFileName()) != externalSolutionPaths.end())
					continue;

				Ref< const Solution > externalSolution = externalDependency->getSolution();
				T_ASSERT (externalSolution);

				Path externalXcodeProjectPath = FileSystem::getInstance().getAbsolutePath(externalSolution->getRootPath() + L"/" + externalSolution->getName() + L".xcodeproj");
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());

				Path relativeFilePath;
				if (!FileSystem::getInstance().getRelativePath(externalXcodeProjectPath, projectPath, relativeFilePath))
					relativeFilePath = externalXcodeProjectPath;

				s << L"\t\t" << FileUids(externalXcodeProjectPath).getFileUid() << L" /* " << externalXcodeProjectPath.getFileName() << L" */ = { isa = PBXFileReference; lastKnownFileType = \"wrapper.pb-project\"; name = \"" << externalXcodeProjectPath.getFileName() << L"\"; path = \"" << relativeFilePath.getPathName() << L"\"; sourceTree = SOURCE_ROOT; };" << Endl;

				externalSolutionPaths.insert(externalDependency->getSolutionFileName());
			}
		}
	}
	s << L"/* End PBXFileReference section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXFrameworksBuildPhase section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		if (!dependencies.empty())
		{
			s << L"\t\t" << ProjectUids(*i).getBuildPhaseFrameworksUid() << L" /* Frameworks */ = {" << Endl;
			s << L"\t\t\tisa = PBXFrameworksBuildPhase;" << Endl;
			s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
			s << L"\t\t\tfiles = (" << Endl;

			for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
				{
					Configuration::TargetFormat targetFormat = getTargetFormat(projectDependency->getProject());
					s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(projectDependency->getProject()) << L" /* " << ProjectUids(projectDependency->getProject()).getProductName(targetFormat) << L" in Frameworks */," << Endl;
				}
				else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
				{
					Configuration::TargetFormat targetFormat = getTargetFormat(externalDependency->getProject());
					s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(externalDependency->getProject()) << L" /* " << ProjectUids(externalDependency->getProject()).getProductName(targetFormat) << L" in Frameworks */," << Endl;
				}
			}

			s << L"\t\t\t);" << Endl;
			s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
			s << L"\t\t};" << Endl;
		}
	}
	s << L"/* End PBXFrameworksBuildPhase section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXGroup section */" << Endl;
	s << L"\t\t" << SolutionUids(solution).getGroupUid() << L" = {" << Endl;
	s << L"\t\t\tisa = PBXGroup;" << Endl;
	s << L"\t\t\tchildren = (" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::wstring projectGroupUid = ProjectUids(*i).getGroupUid();
		s << L"\t\t\t\t" << projectGroupUid << L" /* " << (*i)->getName() << L" */," << Endl;
	}
	s << L"\t\t\t\t" << SolutionUids(solution).getProductsGroupUid() << L" /* Products */," << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
	s << L"\t\t};" << Endl;

	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		createGroups(
			*i,
			s,
			ProjectUids(*i).getGroupUid(),
			(*i)->getName(),
			(*i)->getItems()
		);
	}

	s << L"\t\t" << SolutionUids(solution).getProductsGroupUid() << L" = /* Products */ {" << Endl;
	s << L"\t\t\tisa = PBXGroup;" << Endl;
	s << L"\t\t\tchildren = (" << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tname = Products;" << Endl;
	s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
	s << L"\t\t};" << Endl;

	{
		std::set< const Solution* > externalSolutions;

		for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			const RefList< Dependency >& dependencies = (*i)->getDependencies();
			for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
					externalSolutions.insert(externalDependency->getSolution());
			}
		}

		for (std::set< const Solution* >::const_iterator i = externalSolutions.begin(); i != externalSolutions.end(); ++i)
		{
			s << L"\t\t" << SolutionUids(*i).getProductsGroupUid() << L" = /* Products */ {" << Endl;
			s << L"\t\t\tisa = PBXGroup;" << Endl;
			s << L"\t\t\tchildren = (" << Endl;

			for (RefList< Project >::const_iterator j = projects.begin(); j != projects.end(); ++j)
			{
				const RefList< Dependency >& dependencies = (*j)->getDependencies();
				for (RefList< Dependency >::const_iterator k = dependencies.begin(); k != dependencies.end(); ++k)
				{
					if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*k))
					{
						if (externalDependency->getSolution() != *i)
							continue;

						Configuration::TargetFormat targetFormat = getTargetFormat(externalDependency->getProject());

						std::wstring productUid = ProjectUids(*j).getTargetDependencyUid(externalDependency->getProject());
						std::wstring productName = ProjectUids(externalDependency->getProject()).getProductName(targetFormat);

						s << L"\t\t\t\t" << productUid << L" /* " << productName << L" */," << Endl;
					}
				}
			}

			s << L"\t\t\t);" << Endl;
			s << L"\t\t\tname = Products;" << Endl;
			s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
			s << L"\t\t};" << Endl;
		}
	}

	s << L"/* End PBXGroup section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXNativeTarget section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		s << L"\t\t" << ProjectUids(*i).getTargetUid() << L" /* " << (*i)->getName() << L" */ = {" << Endl;
		s << L"\t\t\tisa = PBXNativeTarget;" << Endl;
		s << L"\t\t\tbuildConfigurationList = " << ProjectUids(*i).getBuildConfigurationListUid() << L" /* Build configuration list for PBXNativeTarget \"" << (*i)->getName() << L"\" */;" << Endl;
		s << L"\t\t\tbuildPhases = (" << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseHeadersUid() << L" /* Headers */," << Endl;
		s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseSourcesUid() << L" /* Sources */," << Endl;

		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		if (!dependencies.empty())
			s << L"\t\t\t\t" << ProjectUids(*i).getBuildPhaseFrameworksUid() << L" /* Frameworks */," << Endl;

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tbuildRules = (" << Endl;
		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tdependencies = (" << Endl;

		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
				s << L"\t\t\t\t" << ProjectUids(*i).getTargetDependencyUid(projectDependency->getProject()) << L" /* PBXTargetDependency */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tname = " << (*i)->getName() << L";" << Endl;
		s << L"\t\t\tproductName = " << (*i)->getName() << L";" << Endl;
		s << L"\t\t\tproductReference = " << ProjectUids(*i).getProductUid() << L" /* " << ProjectUids(*i).getProductName(targetFormat) << L" */;" << Endl;
		s << L"\t\t\tproductType = \"" << ProjectUids(*i).getProductType(targetFormat) << L"\";" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXNativeTarget section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXProject section */" << Endl;
	s << L"\t\t" << SolutionUids(solution).getProjectUid() << L" /* Project object */ = {" << Endl;
	s << L"\t\t\tisa = PBXProject;" << Endl;
	s << L"\t\t\tbuildConfigurationList = " << SolutionUids(solution).getBuildConfigurationListUid() << L" /* Build configuration list for PBXProject \"" << solution->getName() << L"\" */;" << Endl;
	s << L"\t\t\tcompatibilityVersion = \"Xcode 3.1\";" << Endl;
	s << L"\t\t\thasScannedForEncodings = 0;" << Endl;
	s << L"\t\t\tmainGroup = " << SolutionUids(solution).getGroupUid() << L";" << Endl;
	s << L"\t\t\tproductRefGroup = " << SolutionUids(solution).getProductsGroupUid() << L" /* Products */;" << Endl;
	s << L"\t\t\tprojectDirPath = \"\";" << Endl;
	s << L"\t\t\tprojectReferences = (" << Endl;
	
	externalSolutionPaths.clear();
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				if (externalSolutionPaths.find(externalDependency->getSolutionFileName()) != externalSolutionPaths.end())
					continue;

				Ref< const Solution > externalSolution = externalDependency->getSolution();
				T_ASSERT (externalSolution);

				Path externalXcodeProjectPath = externalSolution->getRootPath() + L"/" + externalSolution->getName() + L".xcodeproj";

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
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		s << L"\t\t\t\t" << ProjectUids(*i).getTargetUid() << L" /* " << (*i)->getName() << L" */," << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t};" << Endl;
	s << L"/* End PBXProject section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXReferenceProxy section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			{
				Configuration::TargetFormat targetFormat = getTargetFormat(externalDependency->getProject());

				std::wstring productName = ProjectUids(externalDependency->getProject()).getProductName(targetFormat);

				s << L"\t\t" << ProjectUids(*i).getTargetDependencyUid(externalDependency->getProject()) << L" /* " << productName << L" */ = {" << Endl;
				s << L"\t\t\tisa = PBXReferenceProxy;" << Endl;
				s << L"\t\t\tfileType = archive.ar;" << Endl;
				s << L"\t\t\tpath = \"" << productName << L"\";" << Endl;
				s << L"\t\t\tremoteRef = " << ProjectUids(*i).getContainerItemProxy(externalDependency->getProject()) << L" /* PBXContainerItemProxy */;" << Endl;
				s << L"\t\t\tsourceTree = BUILT_PRODUCTS_DIR;" << Endl;
				s << L"\t\t};" << Endl;
			}
		}
	}
	s << L"/* End PBXReferenceProxy section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXHeadersBuildPhase section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
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

	s << L"/* Begin PBXSourcesBuildPhase section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
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
			if (extension == L"c" || extension == L"cc" || extension == L"cpp" || extension == L"m" || extension == L"mm")
				s << L"\t\t\t\t" << ProjectUids(*i).getBuildFileUid(*j) << L" /* " << j->getFileName() << L" */," << Endl;
		}

		s << L"\t\t\t);" << Endl;
		s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End PBXSourcesBuildPhase section */" << Endl;
	s << Endl;

	s << L"/* Begin PBXTargetDependency section */" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			{
				s << L"\t\t" << ProjectUids(*i).getTargetDependencyUid(projectDependency->getProject()) << L" /* PBXTargetDependency */ = {" << Endl;
				s << L"\t\t\tisa = PBXTargetDependency;" << Endl;
				s << L"\t\t\ttarget = " << ProjectUids(projectDependency->getProject()).getTargetUid() << L" /* " << projectDependency->getProject()->getName() << L" */;" << Endl;
				s << L"\t\t\ttargetProxy = " << ProjectUids(*i).getContainerItemProxy(projectDependency->getProject()) << L" /* PBXContainerItemProxy */;" << Endl;
				s << L"\t\t};" << Endl;
			}
		}
	}
	s << L"/* End PBXTargetDependency section */" << Endl;
	s << Endl;

	s << L"/* Begin XCBuildConfiguration section */" << Endl;
	s << L"\t\t" << SolutionUids(solution).getBuildConfigurationDebugUid() << L" /* Debug */ = {" << Endl;
	s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
	s << L"\t\t\tbuildSettings = {" << Endl;
	s << L"\t\t\t\tCOPY_PHASE_STRIP = NO;" << Endl;
	s << L"\t\t\t};" << Endl;
	s << L"\t\t\tname = Debug;" << Endl;
	s << L"\t\t};" << Endl;
	s << L"\t\t" << SolutionUids(solution).getBuildConfigurationReleaseUid() << L" /* Release */ = {" << Endl;
	s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
	s << L"\t\t\tbuildSettings = {" << Endl;
	s << L"\t\t\t\tCOPY_PHASE_STRIP = NO;" << Endl;
	s << L"\t\t\t};" << Endl;
	s << L"\t\t\tname = Release;" << Endl;
	s << L"\t\t};" << Endl;

	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Configuration > configurations[2];
		getConfigurations(*i, configurations);

		// Debug configuration
		if (configurations[0])
		{
			s << L"\t\t" << ProjectUids(*i).getBuildConfigurationDebugUid() << L" /* " << (*i)->getName() << L" Debug */ = {" << Endl;
			s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
			s << L"\t\t\tbuildSettings = {" << Endl;

			s << L"\t\t\t\tALWAYS_SEARCH_USER_PATHS = YES;" << Endl;
			s << L"\t\t\t\tCOPY_PHASE_STRIP = NO;" << Endl;
			s << L"\t\t\t\tGCC_DYNAMIC_NO_PIC = NO;" << Endl;
			s << L"\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;" << Endl;
			
			s << L"\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"";
			const std::vector< std::wstring >& definitions = configurations[0]->getDefinitions();
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
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());
				Path includePath = FileSystem::getInstance().getAbsolutePath(*j);

				Path relativeIncludePath;
				if (!FileSystem::getInstance().getRelativePath(includePath, projectPath, relativeIncludePath))
					relativeIncludePath = includePath;

				if (j != includePaths.begin())
					s << L" ";
				s << relativeIncludePath.getPathName();
			}
			s << L"\";" << Endl;

			s << L"\t\t\t\tPREBINDING = NO;" << Endl;
			s << L"\t\t\t\tPRODUCT_NAME = " << (*i)->getName() << L";" << Endl;
			s << L"\t\t\t\tUSE_HEADERMAP = NO;" << Endl;

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

			s << L"\t\t\t\tALWAYS_SEARCH_USER_PATHS = YES;" << Endl;
			s << L"\t\t\t\tCOPY_PHASE_STRIP = NO;" << Endl;
			s << L"\t\t\t\tGCC_DYNAMIC_NO_PIC = NO;" << Endl;
			s << L"\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;" << Endl;

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
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());
				Path includePath = FileSystem::getInstance().getAbsolutePath(*j);

				Path relativeIncludePath;
				if (!FileSystem::getInstance().getRelativePath(includePath, projectPath, relativeIncludePath))
					relativeIncludePath = includePath;

				if (j != includePaths.begin())
					s << L" ";
				s << relativeIncludePath.getPathName();
			}
			s << L"\";" << Endl;

			s << L"\t\t\t\tPREBINDING = NO;" << Endl;
			s << L"\t\t\t\tPRODUCT_NAME = " << (*i)->getName() << L";" << Endl;
			s << L"\t\t\t\tUSE_HEADERMAP = NO;" << Endl;

			s << L"\t\t\t};" << Endl;
			s << L"\t\t\tname = Release;" << Endl;
			s << L"\t\t};" << Endl;
		}
	}
	s << L"/* End XCBuildConfiguration section */" << Endl;
	s << Endl;

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
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		s << L"\t\t" << ProjectUids(*i).getBuildConfigurationListUid() << L" /* Build configuration list for PBXNativeTarget \"" << (*i)->getName() << L"\" */ = {" << Endl;
		s << L"\t\t\tisa = XCConfigurationList;" << Endl;
		s << L"\t\t\tbuildConfigurations = (" << Endl;
		
		Ref< Configuration > configurations[2];
		getConfigurations(*i, configurations);
		
		if (configurations[0])
			s << L"\t\t\t\t" <<  ProjectUids(*i).getBuildConfigurationDebugUid() << L" /* Debug */," << Endl;
		if (configurations[1])
			s << L"\t\t\t\t" <<  ProjectUids(*i).getBuildConfigurationReleaseUid() << L" /* Release */," << Endl;
		
		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tdefaultConfigurationIsVisible = 0;" << Endl;
		s << L"\t\t\tdefaultConfigurationName = Debug;" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End XCConfigurationList section */" << Endl;
	s << L"\t};" << Endl;

	s << L"\trootObject = " << SolutionUids(solution).getProjectUid() << L" /* Project object */;" << Endl;
	s << L"}" << Endl;

	s.close();

	return true;
}

void SolutionBuilderXcode::showOptions() const
{
	log::info << L"-d = Debug configuration" << Endl;
	log::info << L"-r = Release configuration" << Endl;
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

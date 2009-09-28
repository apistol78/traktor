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
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Filter.h"

using namespace traktor;

namespace
{

	const wchar_t c_hex[] = { L"0123456789ABCDEF" };

	std::map< std::pair< const Object*, int32_t >, std::wstring > g_objectUids;
	std::map< std::pair< std::wstring, int32_t >, std::wstring > g_fileUids;
	std::map< std::pair< const Object*, std::wstring >, std::wstring > g_objectFileUids;

	std::wstring createNewUid()
	{
		Guid guid = Guid::create();
		StringOutputStream ss;
		for (int i = 0; i < 12; ++i)
		{
			uint8_t p = guid[i];
			ss << c_hex[p >> 4] << c_hex[p & 15];
		}
		return ss.str();
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
					std::wstring childFileUid = calculateUid(*j, -2);
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

	// Global identifiers.
	std::wstring solutionProjectUid = calculateUid(solution, -1);
	std::wstring solutionConfigurationListUid = calculateUid(solution, -2);
	std::wstring solutionGroupUid = calculateUid(solution, -3);
	std::wstring solutionSourcesBuildPhaseUid = calculateUid(solution, -4);
	std::wstring productsGroupUid = calculateUid(solution, -5);

	std::wstring xcodeProjectPath = solution->getRootPath() + L"/" + solution->getName() + L".xcodeproj";

	if (!FileSystem::getInstance().makeDirectory(xcodeProjectPath))
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

	if (!files.empty())
	{
		s << L"/* Begin PBXBuildFile section */" << Endl;
		for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			std::set< Path > projectFiles;
			collectProjectFiles(*i, projectFiles);

			RefList< Configuration > configurations;
			getConfigurations(*i, configurations);
			
			for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
			{
				std::wstring extension = toLower(j->getExtension());
				if (extension == L"c" || extension == L"cc" || extension == L"cpp" || extension == L"m" || extension == L"mm")
				{
					std::wstring fileUid = calculateUid(*j, -1);
					std::wstring buildFileUid = calculateUid(*j, -2);
					s << L"\t\t" << buildFileUid << L" /* " << j->getFileName() << L" in Source */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << j->getFileName() << L" */; };" << Endl;
				}
				else if (extension == L"h" || extension == L"hh" || extension == L"hpp")
				{
					std::wstring fileUid = calculateUid(*j, -1);
					std::wstring buildFileUid = calculateUid(*j, -2);
					s << L"\t\t" << buildFileUid << L" /* " << j->getFileName() << L" in Header */ = { isa = PBXBuildFile; fileRef = " << fileUid << L" /* " << j->getFileName() << L" */; };" << Endl;
				}
			}
		}

		s << L"/* End PBXBuildFile section */" << Endl;
		s << Endl;
	}

	if (!files.empty() && !projects.empty())
	{
		s << L"/* Begin PBXFileReference section */" << Endl;
		for (std::set< Path >::const_iterator i = files.begin(); i != files.end(); ++i)
		{
			std::wstring fileUid = calculateUid(*i, -1);

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

			std::wstring projectTargetListUid = calculateUid(*i, -2);
			std::wstring projectTargetProductUid = calculateUid(*i, -3);
			std::wstring projectTargetProductName;

			switch (targetFormat)
			{
			case Configuration::TfStaticLibrary:
				projectTargetProductName = L"lib" + (*i)->getName() + L".a";
				break;
			case Configuration::TfSharedLibrary:
				projectTargetProductName = L"lib" + (*i)->getName() + L".dylib";
				break;
			case Configuration::TfExecutable:
				projectTargetProductName = (*i)->getName();
				break;
			case Configuration::TfExecutableConsole:
				projectTargetProductName = (*i)->getName();
				break;
			}

			s << L"\t\t" << projectTargetProductUid << L" /* " << projectTargetProductName << L" */ = { isa = PBXFileReference; includeInIndex = 0; path = " << projectTargetProductName << L"; sourceTree = BUILT_PRODUCTS_DIR; };" << Endl;
		}
		s << L"/* End PBXFileReference section */" << Endl;
		s << Endl;
	}

	//s << L"/* Begin PBXFrameworksBuildPhase section */" << Endl;
	//s << L"/* End PBXFrameworksBuildPhase section */" << Endl;
	//s << Endl;

	s << L"/* Begin PBXGroup section */" << Endl;
	s << L"\t\t" << solutionGroupUid << L" = {" << Endl;
	s << L"\t\t\tisa = PBXGroup;" << Endl;
	s << L"\t\t\tchildren = (" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::wstring projectGroupUid = calculateUid(*i, -3);
		s << L"\t\t\t\t" << projectGroupUid << L" /* " << (*i)->getName() << L" */," << Endl;
	}
	s << L"\t\t\t\t" << productsGroupUid << L" /* Products */," << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
	s << L"\t\t};" << Endl;

	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::wstring projectGroupUid = calculateUid(*i, -3);
		createGroups(
			*i,
			s,
			projectGroupUid,
			(*i)->getName(),
			(*i)->getItems()
		);
	}

	s << L"\t\t" << productsGroupUid << L" = /* Products */ {" << Endl;
	s << L"\t\t\tisa = PBXGroup;" << Endl;
	s << L"\t\t\tchildren = (" << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tname = Products;" << Endl;
	s << L"\t\t\tsourceTree = \"<group>\";" << Endl;
	s << L"\t\t};" << Endl;

	s << L"/* End PBXGroup section */" << Endl;
	s << Endl;

	if (!projects.empty())
	{
		s << L"/* Begin PBXNativeTarget section */" << Endl;
		for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			Configuration::TargetFormat targetFormat = getTargetFormat(*i);
			
			std::wstring projectTargetUid = calculateUid(*i, -1);
			std::wstring projectTargetListUid = calculateUid(*i, -2);
			std::wstring projectTargetProductUid = calculateUid(*i, -3);
			std::wstring projectTargetHeadersBuildPhaseUid = calculateUid(*i, -4);
			std::wstring projectTargetSourcesBuildPhaseUid = calculateUid(*i, -5);
			std::wstring projectTargetProductName;
			std::wstring projectTargetProductType;
			
			switch (targetFormat)
			{
			case Configuration::TfStaticLibrary:
				projectTargetProductName = L"lib" + (*i)->getName() + L".a";
				projectTargetProductType = L"com.apple.product-type.library.static";
				break;
			case Configuration::TfSharedLibrary:
				projectTargetProductName = L"lib" + (*i)->getName() + L".dylib";
				projectTargetProductType = L"com.apple.product-type.library.dynamic";
				break;
			case Configuration::TfExecutable:
				projectTargetProductName = (*i)->getName();
				projectTargetProductType = L"com.apple.product-type.application";
				break;
			case Configuration::TfExecutableConsole:
				projectTargetProductName = (*i)->getName();
				projectTargetProductType = L"com.apple.product-type.tool";
				break;
			}
			
			s << L"\t\t" << projectTargetUid << L" /* " << (*i)->getName() << L" */ = {" << Endl;
			s << L"\t\t\tisa = PBXNativeTarget;" << Endl;
			s << L"\t\t\tbuildConfigurationList = " << projectTargetListUid << L" /* Build configuration list for PBXNativeTarget \"" << (*i)->getName() << L"\" */;" << Endl;
			s << L"\t\t\tbuildPhases = (" << Endl;
			s << L"\t\t\t\t" << projectTargetHeadersBuildPhaseUid << L" /* Headers */," << Endl;
			s << L"\t\t\t\t" << projectTargetSourcesBuildPhaseUid << L" /* Sources */," << Endl;
			s << L"\t\t\t);" << Endl;
			s << L"\t\t\tbuildRules = (" << Endl;
			s << L"\t\t\t);" << Endl;
			s << L"\t\t\tdependencies = (" << Endl;
			s << L"\t\t\t);" << Endl;
			s << L"\t\t\tname = " << (*i)->getName() << L";" << Endl;
			s << L"\t\t\tproductName = " << (*i)->getName() << L";" << Endl;
			s << L"\t\t\tproductReference = " << projectTargetProductUid << L" /* " << projectTargetProductName << L" */;" << Endl;
			s << L"\t\t\tproductType = \"" << projectTargetProductType << L"\";" << Endl;
			s << L"\t\t};" << Endl;
		}
		s << L"/* End PBXNativeTarget section */" << Endl;
		s << Endl;
	}

	s << L"/* Begin PBXProject section */" << Endl;
	s << L"\t\t" << solutionProjectUid << L" /* Project object */ = {" << Endl;
	s << L"\t\t\tisa = PBXProject;" << Endl;
	s << L"\t\t\tbuildConfigurationList = " << solutionConfigurationListUid << L" /* Build configuration list for PBXProject \"" << solution->getName() << L"\" */;" << Endl;
	s << L"\t\t\tcompatibilityVersion = \"Xcode 3.1\";" << Endl;
	s << L"\t\t\thasScannedForEncodings = 0;" << Endl;
	s << L"\t\t\tmainGroup = " << solutionGroupUid << L";" << Endl;
	s << L"\t\t\tproductRefGroup = " << productsGroupUid << L" /* Products */;" << Endl;
	s << L"\t\t\tprojectDirPath = \"\";" << Endl;
	s << L"\t\t\tprojectRoot = \"\";" << Endl;
	s << L"\t\t\ttargets = (" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::wstring projectTargetUid = calculateUid(*i, -1);
		s << L"\t\t\t\t" << projectTargetUid << L" /* " << (*i)->getName() << L" */," << Endl;
	}
	s << L"\t\t\t);" << Endl;
	s << L"\t\t};" << Endl;
	s << L"/* End PBXProject section */" << Endl;
	s << Endl;

	if (!projects.empty())
	{
		s << L"/* Begin PBXHeadersBuildPhase section */" << Endl;
		for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			Configuration::TargetFormat targetFormat = getTargetFormat(*i);
			
			std::wstring projectTargetUid = calculateUid(*i, -1);
			std::wstring projectTargetListUid = calculateUid(*i, -2);
			std::wstring projectTargetProductUid = calculateUid(*i, -3);
			std::wstring projectTargetHeadersBuildPhaseUid = calculateUid(*i, -4);
			std::wstring projectTargetSourcesBuildPhaseUid = calculateUid(*i, -5);
			std::wstring projectTargetProductName;
			std::wstring projectTargetProductType;

			s << L"\t\t" << projectTargetHeadersBuildPhaseUid << L" /* Headers */ = {" << Endl;
			s << L"\t\t\tisa = PBXHeadersBuildPhase;" << Endl;
			s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
			s << L"\t\t\tfiles = (" << Endl;

			std::set< Path > projectFiles;
			collectProjectFiles(*i, projectFiles);
			for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
			{
				std::wstring extension = toLower(j->getExtension());
				if (extension == L"h" || extension == L"hh" || extension == L"hpp")
				{
					std::wstring buildFileUid = calculateUid(*j, -2);
					s << L"\t\t\t\t" << buildFileUid << L" /* " << j->getFileName() << L" */," << Endl;
				}
			}

			s << L"\t\t\t);" << Endl;
			s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
			s << L"\t\t};" << Endl;
		}
		s << L"/* End PBXHeadersBuildPhase section */" << Endl;
		s << Endl;
	}

	if (!projects.empty())
	{
		s << L"/* Begin PBXSourcesBuildPhase section */" << Endl;
		for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			Configuration::TargetFormat targetFormat = getTargetFormat(*i);
			
			std::wstring projectTargetUid = calculateUid(*i, -1);
			std::wstring projectTargetListUid = calculateUid(*i, -2);
			std::wstring projectTargetProductUid = calculateUid(*i, -3);
			std::wstring projectTargetHeadersBuildPhaseUid = calculateUid(*i, -4);
			std::wstring projectTargetSourcesBuildPhaseUid = calculateUid(*i, -5);
			std::wstring projectTargetProductName;
			std::wstring projectTargetProductType;

			s << L"\t\t" << projectTargetSourcesBuildPhaseUid << L" /* Sources */ = {" << Endl;
			s << L"\t\t\tisa = PBXSourcesBuildPhase;" << Endl;
			s << L"\t\t\tbuildActionMask = 2147483647;" << Endl;
			s << L"\t\t\tfiles = (" << Endl;

			std::set< Path > projectFiles;
			collectProjectFiles(*i, projectFiles);
			for (std::set< Path >::const_iterator j = projectFiles.begin(); j != projectFiles.end(); ++j)
			{
				std::wstring extension = toLower(j->getExtension());
				if (extension == L"c" || extension == L"cc" || extension == L"cpp" || extension == L"m" || extension == L"mm")
				{
					std::wstring buildFileUid = calculateUid(*j, -2);
					s << L"\t\t\t\t" << buildFileUid << L" /* " << j->getFileName() << L" */," << Endl;
				}
			}

			s << L"\t\t\t);" << Endl;
			s << L"\t\t\trunOnlyForDeploymentPostprocessing = 0;" << Endl;
			s << L"\t\t};" << Endl;
		}
		s << L"/* End PBXSourcesBuildPhase section */" << Endl;
		s << Endl;
	}

	s << L"/* Begin XCBuildConfiguration section */" << Endl;
	std::wstring solutionConfigurationDebugUid = calculateUid(solution, 0);
	std::wstring solutionConfigurationReleaseUid = calculateUid(solution, 1);
	s << L"\t\t" << solutionConfigurationDebugUid << L" /* Debug */ = {" << Endl;
	s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
	s << L"\t\t\tbuildSettings = {" << Endl;
	s << L"\t\t\t\tCOPY_PHASE_STRIP = NO;" << Endl;
	s << L"\t\t\t};" << Endl;
	s << L"\t\t\tname = Debug;" << Endl;
	s << L"\t\t};" << Endl;
	s << L"\t\t" << solutionConfigurationReleaseUid << L" /* Release */ = {" << Endl;
	s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
	s << L"\t\t\tbuildSettings = {" << Endl;
	s << L"\t\t\t\tCOPY_PHASE_STRIP = NO;" << Endl;
	s << L"\t\t\t};" << Endl;
	s << L"\t\t\tname = Release;" << Endl;
	s << L"\t\t};" << Endl;

	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		RefList< Configuration > configurations;
		getConfigurations(*i, configurations);

		for (RefList< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			std::wstring projectTargetConfigurationUid = calculateUid(*j, -5);

			s << L"\t\t" << projectTargetConfigurationUid << L" /* " << (*j)->getName() << L" */ = {" << Endl;
			s << L"\t\t\tisa = XCBuildConfiguration;" << Endl;
			s << L"\t\t\tbuildSettings = {" << Endl;

			s << L"\t\t\t\tALWAYS_SEARCH_USER_PATHS = NO;" << Endl;
			s << L"\t\t\t\tCOPY_PHASE_STRIP = NO;" << Endl;
			s << L"\t\t\t\tGCC_DYNAMIC_NO_PIC = NO;" << Endl;
			s << L"\t\t\t\tGCC_OPTIMIZATION_LEVEL = 0;" << Endl;
			
			s << L"\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = \"";
			const std::vector< std::wstring >& definitions = (*j)->getDefinitions();
			for (std::vector< std::wstring >::const_iterator k = definitions.begin(); k != definitions.end(); ++k)
				s << *k << L";";
			s << L"\";" << Endl;

			s << L"\t\t\t\tHEADER_SEARCH_PATHS = \"";
			const std::vector< std::wstring >& includePaths = (*j)->getIncludePaths();
			for (std::vector< std::wstring >::const_iterator k = includePaths.begin(); k != includePaths.end(); ++k)
			{
				Path projectPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());
				Path includePath = FileSystem::getInstance().getAbsolutePath(*k);

				Path relativeIncludePath;
				if (!FileSystem::getInstance().getRelativePath(includePath, projectPath, relativeIncludePath))
					relativeIncludePath = includePath;

				s << relativeIncludePath.getPathName() << L";";
			}
			s << L"\";" << Endl;

			s << L"\t\t\t\tPREBINDING = NO;" << Endl;
			s << L"\t\t\t\tPRODUCT_NAME = " << (*i)->getName() << L";" << Endl;
			s << L"\t\t\t\tUSE_HEADERMAP = NO;" << Endl;

			s << L"\t\t\t};" << Endl;
			s << L"\t\t\tname = " << (*j)->getName() << L";" << Endl;
			s << L"\t\t};" << Endl;
		}
	}
	s << L"/* End XCBuildConfiguration section */" << Endl;
	s << Endl;

	s << L"/* Begin XCConfigurationList section */" << Endl;
	s << L"\t\t" << solutionConfigurationListUid << L" /* Build configuration list for PBXProject \"" << solution->getName() << L"\" */ = {" << Endl;
	s << L"\t\t\tisa = XCConfigurationList;" << Endl;
	s << L"\t\t\tbuildConfigurations = (" << Endl;
	s << L"\t\t\t\t" << solutionConfigurationDebugUid << L" /* Debug */," << Endl;
	s << L"\t\t\t\t" << solutionConfigurationReleaseUid << L" /* Release */," << Endl;
	s << L"\t\t\t);" << Endl;
	s << L"\t\t\tdefaultConfigurationIsVisible = 0;" << Endl;
	s << L"\t\t\tdefaultConfigurationName = Debug;" << Endl;
	s << L"\t\t};" << Endl;
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Configuration::TargetFormat targetFormat = getTargetFormat(*i);

		std::wstring projectTargetUid = calculateUid(*i, -1);
		std::wstring projectTargetListUid = calculateUid(*i, -2);
		std::wstring projectTargetConfigurationUid = calculateUid(*i, -5);

		s << L"\t\t" << projectTargetListUid << L" /* Build configuration list for PBXNativeTarget \"" << (*i)->getName() << L"\" */ = {" << Endl;
		s << L"\t\t\tisa = XCConfigurationList;" << Endl;
		s << L"\t\t\tbuildConfigurations = (" << Endl;
		
		RefList< Configuration > configurations;
		getConfigurations(*i, configurations);
		
		for (RefList< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			std::wstring projectTargetConfigurationUid = calculateUid(*j, -5);
			s << L"\t\t\t\t" << projectTargetConfigurationUid << L" /* " << (*j)->getName() << L" */," << Endl;
		}
		
		s << L"\t\t\t);" << Endl;
		s << L"\t\t\tdefaultConfigurationIsVisible = 0;" << Endl;
		s << L"\t\t\tdefaultConfigurationName = Debug;" << Endl;
		s << L"\t\t};" << Endl;
	}
	s << L"/* End XCConfigurationList section */" << Endl;
	s << L"\t};" << Endl;

	s << L"\trootObject = " << solutionProjectUid << L" /* Project object */;" << Endl;
	s << L"}" << Endl;

	s.close();

	return true;
}

void SolutionBuilderXcode::showOptions() const
{
	log::info << L"-d = Debug configuration" << Endl;
	log::info << L"-r = Release configuration" << Endl;
}

void SolutionBuilderXcode::getConfigurations(const Project* project, RefList< Configuration >& outConfigurations) const
{
	if (project->getConfiguration(m_debugConfig))
		outConfigurations.push_back(project->getConfiguration(m_debugConfig));
	else
		log::warning << L"Project \"" << project->getName() << L"\" doesn't have debug configuration" << Endl;
		
	if (project->getConfiguration(m_releaseConfig))
		outConfigurations.push_back(project->getConfiguration(m_releaseConfig));
	else
		log::warning << L"Project \"" << project->getName() << L"\" doesn't have release configuration" << Endl;
}

Configuration::TargetFormat SolutionBuilderXcode::getTargetFormat(const Project* project) const
{
	/*
	 Hack #2, get target format from first configuration but it must be the same in all configurations.
	 */
	RefList< Configuration > configurations;
	getConfigurations(project, configurations);
	
	T_ASSERT (!configurations.empty());
	
	Configuration::TargetFormat targetFormat = configurations.front()->getTargetFormat();
	for (RefList< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
		T_ASSERT ((*i)->getTargetFormat() == targetFormat);
	
	return targetFormat;
}

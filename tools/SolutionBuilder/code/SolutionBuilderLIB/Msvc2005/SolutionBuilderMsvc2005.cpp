#include <sstream>
#include <Core/Io/FileSystem.h>
#include <Core/Io/File.h>
#include <Core/Io/Stream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStaticArray.h>
#include <Core/Serialization/MemberComposite.h>
#include <Core/Misc/MD5.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>
#include <Xml/XmlDeserializer.h>
#include "SolutionBuilderLIB/Msvc2005/SolutionBuilderMsvc2005.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/File.h"

using namespace traktor;

namespace
{

	std::wstring generateGuidString(const std::wstring& fileName)
	{
		MD5 md5;
		
		md5.begin();
		md5.feed(fileName.c_str(), int(fileName.length() * sizeof(wchar_t)));
		md5.end();

		const unsigned char* cs = reinterpret_cast< const unsigned char* >(md5.get());

		std::wstringstream ss;
		ss << L"{";
		for (int i = 0; i < 16; ++i)
		{
			wchar_t hex[] =
			{
				L"0123456789ABCDEF"[(cs[i] >> 4) & 0xf],
				L"0123456789ABCDEF"[cs[i] & 0xf]
			};
			ss << hex[0] << hex[1];
			if (i == 3 || i == 5 || i == 7 || i == 9)
				ss << L"-";
		}
		ss << L"}";

		return ss.str();
	}

	std::wstring quotifyString(const std::wstring& str)
	{
		return replaceAll< std::wstring >(str, L"\"", L"&quot;");
	}

}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMsvc2005", SolutionBuilderMsvc2005, SolutionBuilder)
T_IMPLEMENT_RTTI_SERIALIZABLE_COMPOSITE_CLASS(L"SolutionBuilderMsvc2005.Settings", SolutionBuilderMsvc2005, Settings, Serializable)

bool SolutionBuilderMsvc2005::create(const traktor::CommandLine& cmdLine)
{
	Ref< SolutionBuilderMsvc2005::Settings > settings = gc_new< SolutionBuilderMsvc2005::Settings >();
	if (cmdLine.hasOption('p'))
	{
		std::wstring platform = cmdLine.getOption('p').getString();

		traktor::log::info << L"Loading settings \"" << platform << L"\"..." << Endl;

		Ref< Stream > file = FileSystem::getInstance().open(platform, traktor::File::FmRead);
		if (!file)
		{
			traktor::log::error << L"Unable to open platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		settings = xml::XmlDeserializer(file).readObject< SolutionBuilderMsvc2005::Settings >();
		if (!settings)
		{
			traktor::log::error << L"Unable to read platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		file->close();
	}
	m_settings = *settings;
	return true;
}

bool SolutionBuilderMsvc2005::generate(Solution* solution)
{
	std::wstring solutionFileName = solution->getRootPath() + L"/" + solution->getName() + L".sln";

	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath()))
		return false;

	Ref< Stream > file = FileSystem::getInstance().open(
		solutionFileName,
		traktor::File::FmWrite
	);
	if (!file)
		return false;

	FileOutputStream s(file, gc_new< AnsiEncoding >());

	std::wstring solutionGuid = generateGuidString(solutionFileName);

	RefList< Project >& projects = solution->getProjects();
	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Project* project = *i;
		if (!generateProject(solution, project))
		{
			file->close();
			return false;
		}
	}

	s << L"Microsoft Visual Studio Solution File, Format Version 9.00" << Endl;
	s << L"# Visual Studio 2005" << Endl;
	
	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Project* project = *i;

		s << L"Project(\"" << solutionGuid << L"\") = \"" << project->getName() << L"\", \"" << project->getName() << L"\\" << project->getName() << L".vcproj\", \"" << m_projectGuids[project] << L"\"" << Endl;

		// Add local dependencies.
		RefList< Dependency >& dependencies = project->getDependencies();
		if (!dependencies.empty())
		{
			s << L"\tProjectSection(ProjectDependencies) = postProject" << Endl;
			for (RefList< Dependency >::iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!is_a< ProjectDependency >(*j))
					continue;
				Project* dependencyProject = static_cast< ProjectDependency* >(*j)->getProject();
				s << L"\t\t" << m_projectGuids[dependencyProject] << L" = " << m_projectGuids[dependencyProject] << Endl;
			}
			s << L"\tEndProjectSection" << Endl;
		}

		s << L"EndProject" << Endl;
	}

	s << L"Global" << Endl;
	s << L"\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << Endl;

	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Project* project = *i;

		RefList< Configuration >& configurations = project->getConfigurations();
		for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			Configuration* configuration = *j;

			s << L"\t\t" << m_projectGuids[project] << L"." << configuration->getName() << L"|" << m_settings.platform << L".ActiveCfg = " << configuration->getName() << L"|" << m_settings.platform << Endl;
			s << L"\t\t" << m_projectGuids[project] << L"." << configuration->getName() << L"|" << m_settings.platform << L".Build.0 = " << configuration->getName() << L"|" << m_settings.platform << Endl;
		}
	}

	s << L"\tEndGlobalSection" << Endl;
	s << L"\tGlobalSection(SolutionProperties) = preSolution" << Endl;
	s << L"\t\tHideSolutionNode = FALSE" << Endl;
	s << L"\tEndGlobalSection" << Endl;
	s << L"EndGlobal" << Endl;

	s.close();

	return true;
}

void SolutionBuilderMsvc2005::showOptions() const
{
	traktor::log::info << L"\t-p=[platform]	(Platform description file)" << Endl;
}

bool SolutionBuilderMsvc2005::generateProject(Solution* solution, Project* project)
{
	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath() + L"/" + project->getName()))
		return false;

	std::wstring projectFileName = solution->getRootPath() + L"/" + project->getName() + L"/" + project->getName() + L".vcproj";

	traktor::log::info << L"Generating project \"" << projectFileName << L"\"" << Endl;

	Ref< Stream > file = FileSystem::getInstance().open(
		projectFileName,
		traktor::File::FmWrite
	);
	if (!file)
		return false;

	FileOutputStream s(file, gc_new< AnsiEncoding >());

	m_projectGuids[project] = generateGuidString(projectFileName);

	s << L"<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << Endl;
	s << L"<VisualStudioProject" << Endl;
	s << L"\tProjectType=\"Visual C++\"" << Endl;
	s << L"\tVersion=\"8.00\"" << Endl;
	s << L"\tName=\"" << project->getName() << L"\"" << Endl;
	s << L"\tProjectGUID=\"" << m_projectGuids[project] << L"\"" << Endl;
	s << L"\tRootnamespace=\"" << project->getName() << L"\"" << Endl;
	s << L"\tKeyword=\"" << m_settings.keyword << L"\"" << Endl;
	s << L"\t>" << Endl;

	s << L"\t<Platforms>" << Endl;
	s << L"\t\t<Platform" << Endl;
	s << L"\t\t\tName=\"" << m_settings.platform << L"\"" << Endl;
	s << L"\t\t/>" << Endl;
	s << L"\t</Platforms>" << Endl;

	s << L"\t<ToolFiles/>" << Endl;

	s << L"\t<Configurations>" << Endl;

	RefList< Configuration >& configurations = project->getConfigurations();
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		Configuration* configuration = *i;
		const SettingsTarget& settingsTarget = m_settings.targets[int(configuration->getTargetFormat())];

		s << L"\t\t<Configuration" << Endl;
		s << L"\t\t\tName=\"" << configuration->getName() << L"|" << m_settings.platform << L"\"" << Endl;
		s << L"\t\t\tOutputDirectory=\"$(SolutionDir)$(ConfigurationName)\"" << Endl;
		s << L"\t\t\tIntermediateDirectory=\"$(ConfigurationName)\"" << Endl;

		switch (configuration->getTargetFormat())
		{
		case Configuration::TfStaticLibrary:
			s << L"\t\t\tConfigurationType=\"4\"" << Endl;
			break;

		case Configuration::TfSharedLibrary:
			s << L"\t\t\tConfigurationType=\"2\"" << Endl;
			break;

		case Configuration::TfExecutable:
		case Configuration::TfExecutableConsole:
			s << L"\t\t\tConfigurationType=\"1\"" << Endl;
			break;
		}

		s << L"\t\t\tCharacterSet=\"" << settingsTarget.characterSet << L"\"" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpRelease)
			s << L"\t\t\tWholeProgramOptimization=\"1\"" << Endl;

		s << L"\t\t\t>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCPreBuildEventTool\"" << Endl;
		if (!settingsTarget.preBuildEvent.empty())
			s << L"\t\t\t\tCommandLine=\"" << quotifyString(settingsTarget.preBuildEvent) << L"\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCCustomBuildTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCXMLDataGeneratorTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCMIDLTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"" << settingsTarget.compilerTool << L"\"" << Endl;
		
		std::wstring additionalCompilerOptions;
		if (configuration->getTargetProfile() == Configuration::TpDebug)
			additionalCompilerOptions = settingsTarget.additionalCompilerOptionsDebug;
		if (configuration->getTargetProfile() == Configuration::TpRelease)
			additionalCompilerOptions = settingsTarget.additionalCompilerOptionsRelease;

		if (!additionalCompilerOptions.empty())
			s << L"\t\t\t\tAdditionalOptions=\"" << additionalCompilerOptions << L"\"" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpDebug)
			s << L"\t\t\t\tOptimization=\"0\"" << Endl;
		else
		{
			s << L"\t\t\t\tOptimization=\"3\"" << Endl;
			s << L"\t\t\t\tInlineFunctionExpansion=\"2\"" << Endl;
			s << L"\t\t\t\tEnableIntrinsicFunctions=\"true\"" << Endl;
			s << L"\t\t\t\tFavorSizeOrSpeed=\"1\"" << Endl;
			s << L"\t\t\t\tOmitFramePointers=\"true\"" << Endl;
		}

		s << L"\t\t\t\tAdditionalIncludeDirectories=\"";
		for (std::vector< std::wstring >::const_iterator i = configuration->getIncludePaths().begin(); i != configuration->getIncludePaths().end(); ++i)
		{
			if (i != configuration->getIncludePaths().begin())
				s << L";";
			s << resolvePath(*i);
		}
		s << L"\"" << Endl;

		s << L"\t\t\t\tPreprocessorDefinitions=\"";
		for (std::vector< std::wstring >::const_iterator i = configuration->getDefinitions().begin(); i != configuration->getDefinitions().end(); ++i)
			s << *i << L";";

		s << settingsTarget.definitions;

		switch (configuration->getTargetFormat())
		{
		case Configuration::TfStaticLibrary:
			s << L";_LIBRARY";
			break;

		case Configuration::TfSharedLibrary:
			s << L";_USRDLL";
			break;
		}
		s << L"\"" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpDebug)
			s << L"\t\t\t\tMinimalRebuild=\"true\"" << Endl;

		s << L"\t\t\t\tBasicRuntimeChecks=\"0\"" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpDebug)
			s << L"\t\t\t\tRuntimeLibrary=\"" << settingsTarget.runtimeLibraryDebug << L"\"" << Endl;
		else
			s << L"\t\t\t\tRuntimeLibrary=\"" << settingsTarget.runtimeLibraryRelease << L"\"" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpRelease)
			s << L"\t\t\t\tBufferSecurityCheck=\"false\"" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpRelease)
			s << L"\t\t\t\tFloatingPointModel=\"2\"" << Endl;

		if (configuration->getTargetProfile() == Configuration::TpDebug)
			s << L"\t\t\t\tEnableFunctionLevelLinking=\"true\"" << Endl;

		s << L"\t\t\t\tRuntimeTypeInfo=\"false\"" << Endl;

		std::wstring precompiledHeader = configuration->getPrecompiledHeader();
		if (!precompiledHeader.empty())
		{
			s << L"\t\t\t\tUsePrecompiledHeader=\"2\"" << Endl;
			s << L"\t\t\t\tPrecompiledHeaderThrough=\"" << precompiledHeader << L"\"" << Endl;
		}
		else
		{
			s << L"\t\t\t\tUsePrecompiledHeader=\"0\"" << Endl;
		}

		s << L"\t\t\t\tWarningLevel=\"3\"" << Endl;
		s << L"\t\t\t\tDetect64BitPortabilityProblems=\"false\"" << Endl;
		
		if (configuration->getTargetProfile() == Configuration::TpDebug)
			s << L"\t\t\t\tDebugInformationFormat=\"3\"" << Endl;

		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCManagedResourceCompilerTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCResourceCompilerTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCPreLinkEventTool\"" << Endl;
		if (!settingsTarget.preLinkEvent.empty())
			s << L"\t\t\t\tCommandLine=\"" << quotifyString(settingsTarget.preLinkEvent) << L"\"" << Endl;
		s << L"\t\t\t/>" << Endl;

		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCLibrarianTool\"" << Endl;

		if (configuration->getTargetFormat() == Configuration::TfStaticLibrary)
		{
			if (configuration->getTargetProfile() == Configuration::TpDebug)
				s << L"\t\t\t\tOutputFile=\"$(OutDir)/" << project->getName() << L"_d.lib\"" << Endl;
			else
				s << L"\t\t\t\tOutputFile=\"$(OutDir)/" << project->getName() << L".lib\"" << Endl;
		}

		s << L"\t\t\t/>" << Endl;

		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"" << settingsTarget.linkerTool << L"\"" << Endl;

		if (configuration->getTargetFormat() != Configuration::TfStaticLibrary)
		{
			if (!settingsTarget.machine.empty())
				s << L"\t\t\t\tAdditionalOptions=\"/machine:" << settingsTarget.machine << L" " << settingsTarget.additionalLinkerOptions << L"\"" << Endl;
			s << L"\t\t\t\tAdditionalDependencies=\"" << settingsTarget.additionalDependencies;

			std::set< std::wstring > additionalLibraries;
			std::set< std::wstring > additionalLibraryPaths;

			collectAdditionalLibraries(
				project,
				configuration,
				additionalLibraries,
				additionalLibraryPaths
			);

			for (std::set< std::wstring >::const_iterator i = additionalLibraries.begin(); i != additionalLibraries.end(); ++i)
			{
				if (i != additionalLibraries.begin() || !settingsTarget.additionalDependencies.empty())
					s << L" ";
				s << resolvePath(*i);
			}
			s << L"\"" << Endl;

			switch (configuration->getTargetFormat())
			{
			case Configuration::TfSharedLibrary:
				if (configuration->getTargetProfile() == Configuration::TpDebug)
					s << L"\t\t\t\tOutputFile=\"$(OutDir)/" << project->getName() << L"_d.dll\"" << Endl;
				else
					s << L"\t\t\t\tOutputFile=\"$(OutDir)/" << project->getName() << L".dll\"" << Endl;
				break;

			case Configuration::TfExecutable:
				if (configuration->getTargetProfile() == Configuration::TpDebug)
					s << L"\t\t\t\tOutputFile=\"$(OutDir)/" << project->getName() << L"_d.exe\"" << Endl;
				else
					s << L"\t\t\t\tOutputFile=\"$(OutDir)/" << project->getName() << L".exe\"" << Endl;
				break;
			}

			if (configuration->getTargetProfile() == Configuration::TpDebug)
				s << L"\t\t\t\tLinkIncremental=\"2\"" << Endl;
			else
				s << L"\t\t\t\tLinkIncremental=\"1\"" << Endl;

			s << L"\t\t\t\tAdditionalLibraryDirectories=\"";
			for (std::set< std::wstring >::const_iterator i = additionalLibraryPaths.begin(); i != additionalLibraryPaths.end(); ++i)
			{
				if (i != additionalLibraryPaths.begin())
					s << L";";
				s << resolvePath(*i);
			}
			s << L"\"" << Endl;
			
			if (configuration->getTargetProfile() == Configuration::TpDebug)
			{
				s << L"\t\t\t\tGenerateDebugInformation=\"true\"" << Endl;
				s << L"\t\t\t\tProgramDatabaseFile=\"$(OutDir)/" << project->getName() << L"_d.pdb\"" << Endl;
			}
			else
				s << L"\t\t\t\tGenerateDebugInformation=\"false\"" << Endl;

			s << L"\t\t\t\tSubSystem=\"" << settingsTarget.subsystem << L"\"" << Endl;

			if (configuration->getTargetProfile() == Configuration::TpRelease)
				s << L"\t\t\t\tOptimizeReferences=\"2\"" << Endl;

			if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
			{
				if (configuration->getTargetProfile() == Configuration::TpDebug)
					s << L"\t\t\t\tImportLibrary=\"$(OutDir)/" << project->getName() << L"_d.lib\"" << Endl;
				else
					s << L"\t\t\t\tImportLibrary=\"$(OutDir)/" << project->getName() << L".lib\"" << Endl;
			}
		}

		s << L"\t\t\t/>" << Endl;

		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCALinkTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;

		if (configuration->getTargetFormat() != Configuration::TfStaticLibrary && !settingsTarget.additionalManifestFiles.empty())
		{
			s << L"\t\t\t<Tool" << Endl;
			s << L"\t\t\t\tName=\"VCManifestTool\"" << Endl;
			s << L"\t\t\t\tAdditionalManifestFiles=\"" << settingsTarget.additionalManifestFiles << L"\"" << Endl;
			s << L"\t\t\t\tEmbedManifest=\"true\"" << Endl;
			s << L"\t\t\t/>" << Endl;
		}

		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCXDCMakeTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCBscMakeTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCFxCopTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCAppVerifierTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCWebDeploymentTool\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t\t<Tool" << Endl;
		s << L"\t\t\t\tName=\"VCPostBuildEventTool\"" << Endl;
		if (!settingsTarget.postBuildEvent.empty())
			s << L"\t\t\t\tCommandLine=\"" << quotifyString(settingsTarget.postBuildEvent) << L"\"" << Endl;
		s << L"\t\t\t/>" << Endl;
		s << L"\t\t</Configuration>" << Endl;
	}

	s << L"\t</Configurations>" << Endl;

	s << L"\t<References>" << Endl;
	s << L"\t</References>" << Endl;

	s << L"\t<Files>" << Endl;

	RefList< ProjectItem >& items = project->getItems();
	for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
		addItem(s, solution, project, *i);

	s << L"\t</Files>" << Endl;

	s << L"</VisualStudioProject>" << Endl;

	s.close();

	return true;
}

void SolutionBuilderMsvc2005::addItem(
	OutputStream& s,
	Solution* solution,
	Project* project,
	ProjectItem* item
)
{
	Filter* filter = dynamic_type_cast< Filter* >(item);
	if (filter)
	{
		s << L"\t\t<Filter" << Endl;
		s << L"\t\t\tName=\"" << filter->getName() << L"\"" << Endl;
		s << L"\t\t\t>" << Endl;

		RefList< ProjectItem >& items = item->getItems();
		for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
			addItem(s, solution, project, *i);

		s << L"\t\t</Filter>" << Endl;
	}

	::File* file = dynamic_type_cast< ::File* >(item);
	if (file)
	{
		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);

		Path rootPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());

		for (std::set< Path >::iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
		{
			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				*i,
				rootPath,
				relativePath
			);

			s << L"\t\t<File" << Endl;
			s << L"\t\t\tRelativePath=\"..\\" << replaceAll< std::wstring >(relativePath, '/', '\\') << L"\"" << Endl;
			s << L"\t\t/>" << Endl;
		}
	}
}

void SolutionBuilderMsvc2005::collectAdditionalLibraries(
	Project* project,
	Configuration* configuration,
	std::set< std::wstring >& additionalLibraries,
	std::set< std::wstring >& additionalLibraryPaths
)
{
	additionalLibraries.insert(
		configuration->getLibraries().begin(),
		configuration->getLibraries().end()
	);

	additionalLibraryPaths.insert(
		configuration->getLibraryPaths().begin(),
		configuration->getLibraryPaths().end()
	);

	RefList< Dependency >& dependencies = project->getDependencies();
	for (RefList< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		// Traverse all static library dependencies and at their "additional libraries" as well.
		if (ProjectDependency* projectDependency = dynamic_type_cast< ProjectDependency* >(*i))
		{
			Configuration* dependentConfiguration = projectDependency->getProject()->getConfiguration(configuration->getName());
			if (!dependentConfiguration)
			{
				traktor::log::warning << L"Unable to add dependency \"" << projectDependency->getProject()->getName() << L"\", no matching configuration found" << Endl;
				continue;
			}

			if (dependentConfiguration->getTargetFormat() == Configuration::TfStaticLibrary)
			{
				collectAdditionalLibraries(
					projectDependency->getProject(),
					dependentConfiguration,
					additionalLibraries,
					additionalLibraryPaths
				);
			}
		}

		// Add products from external dependencies and their "additional libraries" as well.
		if (ExternalDependency* externalDependency = dynamic_type_cast< ExternalDependency* >(*i))
		{
			Ref< Configuration > externalConfiguration = externalDependency->getProject()->getConfiguration(configuration->getName());
			if (!externalConfiguration)
			{
				traktor::log::warning << L"Unable to add external dependency \"" << externalDependency->getProject()->getName() << L"\", no matching configuration found" << Endl;
				continue;
			}

			std::wstring externalRootPath = externalDependency->getSolution()->getRootPath();
			std::wstring externalProjectPath = externalRootPath + L"/" + toLower(externalConfiguration->getName());
			std::wstring externalProjectName = externalDependency->getProject()->getName() + ((configuration->getTargetProfile() == Configuration::TpDebug) ? L"_d.lib" : L".lib");

			additionalLibraries.insert(externalProjectName);
			additionalLibraryPaths.insert(externalProjectPath);

			if (externalConfiguration->getTargetFormat() == Configuration::TfStaticLibrary)
			{
				collectAdditionalLibraries(
					externalDependency->getProject(),
					externalConfiguration,
					additionalLibraries,
					additionalLibraryPaths
				);
			}
		}
	}
}

std::wstring SolutionBuilderMsvc2005::resolvePath(const std::wstring& path) const
{
	if (m_settings.resolvePaths)
		return Path(path).c_str();
	else
		return path;
}

SolutionBuilderMsvc2005::SettingsTarget::SettingsTarget()
:	compilerTool(L"VCCLCompilerTool")
,	linkerTool(L"VCLinkerTool")
,	definitions(L"WIN32;_WINDOWS")
,	additionalCompilerOptionsDebug(L"")
,	additionalCompilerOptionsRelease(L"")
,	additionalLinkerOptions(L"")
,	additionalDependencies(L"")
,	subsystem(2)
,	machine(L"X86")
,	characterSet(2)
,	runtimeLibraryDebug(3)
,	runtimeLibraryRelease(2)
,	additionalManifestFiles(L"")
,	preBuildEvent(L"")
,	preLinkEvent(L"")
,	postBuildEvent(L"")
{
}

bool SolutionBuilderMsvc2005::SettingsTarget::serialize(traktor::Serializer& s)
{
	s >> Member< std::wstring >(L"compilerTool", compilerTool);
	s >> Member< std::wstring >(L"linkerTool", linkerTool);
	s >> Member< std::wstring >(L"definitions", definitions);
	s >> Member< std::wstring >(L"additionalCompilerOptionsDebug", additionalCompilerOptionsDebug);
	s >> Member< std::wstring >(L"additionalCompilerOptionsRelease", additionalCompilerOptionsRelease);
	s >> Member< std::wstring >(L"additionalLinkerOptions", additionalLinkerOptions);
	s >> Member< std::wstring >(L"additionalDependencies", additionalDependencies);
	s >> Member< std::wstring >(L"machine", machine);
	s >> Member< int >(L"subsystem", subsystem);
	s >> Member< int >(L"characterSet", characterSet);
	s >> Member< int >(L"runtimeLibraryDebug", runtimeLibraryDebug);
	s >> Member< int >(L"runtimeLibraryRelease", runtimeLibraryRelease);
	s >> Member< std::wstring >(L"additionalManifestFiles", additionalManifestFiles);
	s >> Member< std::wstring >(L"preBuildEvent", preBuildEvent);
	s >> Member< std::wstring >(L"preLinkEvent", preLinkEvent);
	s >> Member< std::wstring >(L"postBuildEvent", postBuildEvent);
	return true;
}

SolutionBuilderMsvc2005::Settings::Settings()
:	platform(L"Win32")
,	keyword(L"Win32Proj")
,	resolvePaths(false)
{
}

bool SolutionBuilderMsvc2005::Settings::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"platform", platform);
	s >> Member< std::wstring >(L"keyword", keyword);
	s >> Member< bool >(L"resolvePaths", resolvePaths);
	s >> MemberStaticArray< SettingsTarget, 4, MemberComposite< SettingsTarget > >(L"targets", targets);
	return true;
}

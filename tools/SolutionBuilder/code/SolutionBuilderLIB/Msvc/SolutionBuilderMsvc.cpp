#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Misc/MD5.h>
#include <Core/Log/Log.h>
#include <Xml/XmlDeserializer.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcSettings.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcProject.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/Configuration.h"

// Forced references
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCProj.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcCompilerTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcGenericTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcLibrarianTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcLinkerTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXProj.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXBuildTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXCustomBuildTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXDefinition.h"

using namespace traktor;

namespace
{

	std::wstring quotifyString(const std::wstring& str)
	{
		return replaceAll< std::wstring >(str, L"\"", L"&quot;");
	}

}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMsvc", SolutionBuilderMsvc, SolutionBuilder)

bool SolutionBuilderMsvc::create(const CommandLine& cmdLine)
{
	Ref< SolutionBuilderMsvcSettings > settings = new SolutionBuilderMsvcSettings();
	if (cmdLine.hasOption('p'))
	{
		std::wstring platform = cmdLine.getOption('p').getString();

		traktor::log::info << L"Loading settings \"" << platform << L"\"..." << Endl;

		Ref< IStream > file = FileSystem::getInstance().open(platform, traktor::File::FmRead);
		if (!file)
		{
			traktor::log::error << L"Unable to open platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		settings = xml::XmlDeserializer(file).readObject< SolutionBuilderMsvcSettings >();
		if (!settings)
		{
			traktor::log::error << L"Unable to read platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		file->close();
	}
	m_settings = settings;
	return true;
}

bool SolutionBuilderMsvc::generate(Solution* solution)
{
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCProj)
	T_FORCE_LINK_REF(SolutionBuilderMsvcCompilerTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcGenericTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcLibrarianTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcLinkerTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXProj)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXBuildTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXCustomBuildTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXDefinition)

	std::wstring solutionFileName = solution->getRootPath() + L"/" + solution->getName() + L".sln";

	// Setup generator context.
	GeneratorContext context;
	context.set(L"SOLUTION_NAME", solution->getName());
	context.set(L"SOLUTION_ROOTPATH", solution->getRootPath());
	context.set(L"SOLUTION_FILENAME", solutionFileName);

	// Create root path.
	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath()))
		return false;

	// Generate projects.
	std::map< const Project*, std::wstring > projectGuids;

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Project > project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath, projectFileName, projectGuid;
		if (!m_settings->getProject()->getInformation(
			context,
			solution,
			project,
			projectPath,
			projectFileName,
			projectGuid
		))
			return false;

		if (!m_settings->getProject()->generate(
			context,
			solution,
			project
		))
			return false;

		projectGuids[project] = projectGuid;
	}

	// Generate solution.
	std::wstring solutionGuid = context.generateGUID(solutionFileName);

	Ref< IStream > file = FileSystem::getInstance().open(
		solutionFileName,
		traktor::File::FmWrite
	);
	if (!file)
		return false;

	FileOutputStream os(file, new AnsiEncoding());

	os << L"Microsoft Visual Studio Solution File, Format Version " << m_settings->getSLNVersion() << Endl;
	os << L"# Visual Studio " << m_settings->getVSVersion() << Endl;
	
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Project > project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath, projectFileName, projectGuid;
		if (!m_settings->getProject()->getInformation(
			context,
			solution,
			project,
			projectPath,
			projectFileName,
			projectGuid
		))
			return false;

		std::wstring projectExtension = Path(projectFileName).getExtension();
		os << L"Project(\"" << solutionGuid << L"\") = \"" << project->getName() << L"\", \"" << project->getName() << L"\\" << project->getName() << L"." << projectExtension << L"\", \"" << projectGuids[project] << L"\"" << Endl;

		// Add local dependencies.
		const RefArray< Dependency >& dependencies = project->getDependencies();
		if (!dependencies.empty())
		{
			os << IncreaseIndent;
			os << L"ProjectSection(ProjectDependencies) = postProject" << Endl;
			for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!is_a< ProjectDependency >(*j))
					continue;

				Ref< const Project > dependencyProject = static_cast< const ProjectDependency* >(*j)->getProject();
				if (!dependencyProject->getEnable())
				{
					traktor::log::warning << L"Trying to add disabled dependency to project \"" << project->getName() << L"\"; dependency skipped" << Endl;
					continue;
				}

				os << IncreaseIndent;
				os << projectGuids[dependencyProject] << L" = " << projectGuids[dependencyProject] << Endl;
				os << DecreaseIndent;
			}
			os << L"EndProjectSection" << Endl;
			os << DecreaseIndent;
		}

		os << L"EndProject" << Endl;
	}

	os << L"Global" << Endl;
	os << IncreaseIndent;
	os << L"GlobalSection(SolutionConfigurationPlatforms) = preSolution" << Endl;

	std::wstring platform = m_settings->getProject()->getPlatform();

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Project > project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		const RefArray< Configuration >& configurations = project->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			Ref< const Configuration > configuration = *j;

			os << IncreaseIndent;
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".ActiveCfg = " << configuration->getName() << L"|" << platform << L"" << Endl;
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".Build.0 = " << configuration->getName() << L"|" << platform << L"" << Endl;
			os << DecreaseIndent;
		}
	}

	os << L"EndGlobalSection" << Endl;
	os << DecreaseIndent;

	os << IncreaseIndent;
	os << L"GlobalSection(SolutionProperties) = preSolution" << Endl;
	os << IncreaseIndent;
	os << L"HideSolutionNode = FALSE" << Endl;
	os << DecreaseIndent;
	os << L"EndGlobalSection" << Endl;
	os << DecreaseIndent;
	os << L"EndGlobal" << Endl;

	os.close();

	return true;
}

void SolutionBuilderMsvc::showOptions() const
{
	traktor::log::info << L"\t-p=[platform]	(Platform description file)" << Endl;
}

#include <Core/RefSet.h>
#include <Core/Log/Log.h>
#include <Core/Misc/MD5.h>
#include <Core/Misc/String.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Xml/XmlDeserializer.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcProject.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcSettings.h"

// Forced references
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCProj.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcCompilerTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcGenericTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcLibrarianTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcLinkerTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcManifestTool.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXProj.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXProjVGDB.h"
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

	struct ProjectNamePredicate
	{
		bool operator () (const Project* p0, const Project* p1) const
		{
			return compareIgnoreCase(p0->getName(), p1->getName()) < 0;
		}
	};

}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMsvc", SolutionBuilderMsvc, SolutionBuilder)

SolutionBuilderMsvc::SolutionBuilderMsvc()
:	m_includeExternal(false)
{
}

bool SolutionBuilderMsvc::create(const CommandLine& cmdLine)
{
	Ref< SolutionBuilderMsvcSettings > settings = new SolutionBuilderMsvcSettings();

	if (cmdLine.hasOption('p', L"msvc-platform"))
	{
		std::wstring platform = cmdLine.getOption('p', L"msvc-platform").getString();

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

	if (cmdLine.hasOption('i', L"msvc-include-external"))
		m_includeExternal = true;

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
	T_FORCE_LINK_REF(SolutionBuilderMsvcManifestTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXProj)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXProjVGDB)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXBuildTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXCustomBuildTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXDefinition)

	std::wstring solutionFileName = solution->getRootPath() + L"/" + solution->getName() + L".sln";

	// Setup generator context.
	GeneratorContext context(m_includeExternal);
	context.set(L"SOLUTION_NAME", solution->getName());
	context.set(L"SOLUTION_ROOTPATH", solution->getRootPath());
	context.set(L"SOLUTION_FILENAME", solutionFileName);

	// Create root path.
	if (!FileSystem::getInstance().makeAllDirectories(solution->getRootPath()))
		return false;

	// Generate projects.
	std::map< const Project*, std::wstring > projectGuids;
	RefSet< Solution > externalSolutions;

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

		// Collect external solutions which we should include. Only include first level of external
		// solutions.
		if (m_includeExternal)
		{
			const RefArray< Dependency >& dependencies = project->getDependencies();
			for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j);
				if (!externalDependency)
					continue;

				Solution* externalSolution = externalDependency->getSolution();
				T_ASSERT (externalSolution);

				const RefArray< Project >& externalProjects = externalSolution->getProjects();
				for (RefArray< Project >::const_iterator j = externalProjects.begin(); j != externalProjects.end(); ++j)
				{
					Project* externalProject = *j;

					if (!externalProject->getEnable())
						continue;

					std::wstring projectPath, projectFileName, projectGuid;
					if (!m_settings->getProject()->getInformation(
						context,
						externalSolution,
						externalProject,
						projectPath,
						projectFileName,
						projectGuid
					))
						return false;

					projectGuids[externalProject] = projectGuid;
				}

				externalSolutions.insert(externalSolution);
			}
		}

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
		os << L"Project(\"" << solutionGuid << L"\") = \"" << project->getName() << L"\", \"" << project->getName() << L"\\" << project->getName() << L"." << projectExtension << L"\", \"" << /*projectGuids[project]*/projectGuid << L"\"" << Endl;

		// Add local, or first order external if enabled, dependencies.
		const RefArray< Dependency >& dependencies = project->getDependencies();
		if (!dependencies.empty())
		{
			os << IncreaseIndent;
			os << L"ProjectSection(ProjectDependencies) = postProject" << Endl;

			for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
				{
					Ref< const Project > dependentProject = projectDependency->getProject();
					if (!dependentProject->getEnable())
					{
						traktor::log::warning << L"Trying to add disabled dependency to project \"" << project->getName() << L"\"; dependency skipped" << Endl;
						continue;
					}

					os << IncreaseIndent;
					os << projectGuids[dependentProject] << L" = " << projectGuids[dependentProject] << Endl;
					os << DecreaseIndent;
				}
				else if (m_includeExternal)
				{
					if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
					{
						Ref< const Project > dependentProject = externalDependency->getProject();
						if (!dependentProject->getEnable())
						{
							traktor::log::warning << L"Trying to add disabled dependency to project \"" << project->getName() << L"\"; dependency skipped" << Endl;
							continue;
						}

						os << IncreaseIndent;
						os << projectGuids[dependentProject] << L" = " << projectGuids[dependentProject] << Endl;
						os << DecreaseIndent;
					}
				}
			}

			os << L"EndProjectSection" << Endl;
			os << DecreaseIndent;
		}

		os << L"EndProject" << Endl;
	}

	// Create solution folder with all external solutions.
	if (m_includeExternal && !externalSolutions.empty())
	{
		for (RefSet< Solution >::const_iterator i = externalSolutions.begin(); i != externalSolutions.end(); ++i)
		{
			Solution* externalSolution = *i;

			os << L"Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"" << externalSolution->getName() << L"\", \"" << externalSolution->getName() << L"\", \"{3E0BD3A9-E831-4928-9EAA-B07D1ED32784}\"" << Endl;
			os << L"EndProject" << Endl;

			RefArray< Project > externalProjects = externalSolution->getProjects();
			externalProjects.sort(ProjectNamePredicate());

			for (RefArray< Project >::const_iterator j = externalProjects.begin(); j != externalProjects.end(); ++j)
			{
				Project* externalProject = *j;

				if (!externalProject->getEnable())
					continue;

				std::wstring projectPath, projectFileName, projectGuid;
				if (!m_settings->getProject()->getInformation(
					context,
					externalSolution,
					externalProject,
					projectPath,
					projectFileName,
					projectGuid
				))
					continue;

				Path externalProjectPath;
				FileSystem::getInstance().getRelativePath(projectFileName, solution->getRootPath(), externalProjectPath);

				std::wstring projectExtension = Path(projectFileName).getExtension();
				os << L"Project(\"" << solutionGuid << L"\") = \"" << externalProject->getName() << L"\", \"" << replaceAll(externalProjectPath.getPathName(), L'/', L'\\') << L"\", \"" << projectGuid << L"\"" << Endl;

				const RefArray< Dependency >& dependencies = externalProject->getDependencies();
				if (!dependencies.empty())
				{
					os << IncreaseIndent;
					os << L"ProjectSection(ProjectDependencies) = postProject" << Endl;

					for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
					{
						if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
						{
							Ref< const Project > dependentProject = projectDependency->getProject();
							if (!dependentProject->getEnable())
							{
								traktor::log::warning << L"Trying to add disabled dependency to project \"" << externalProject->getName() << L"\"; dependency skipped" << Endl;
								continue;
							}

							os << IncreaseIndent;
							os << projectGuids[dependentProject] << L" = " << projectGuids[dependentProject] << Endl;
							os << DecreaseIndent;
						}
						else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
						{
							Ref< const Project > dependentProject = externalDependency->getProject();
							if (!dependentProject->getEnable())
							{
								traktor::log::warning << L"Trying to add disabled dependency to project \"" << externalProject->getName() << L"\"; dependency skipped" << Endl;
								continue;
							}

							os << IncreaseIndent;
							os << projectGuids[dependentProject] << L" = " << projectGuids[dependentProject] << Endl;
							os << DecreaseIndent;
						}
					}

					os << L"EndProjectSection" << Endl;
					os << DecreaseIndent;
				}

				os << L"EndProject" << Endl;
			}
		}
	}

	os << L"Global" << Endl;
	os << IncreaseIndent;

	std::wstring platform = m_settings->getProject()->getPlatform();

	std::set< std::wstring > availableConfigurations;
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
			availableConfigurations.insert(configuration->getName());
		}
	}

	os << L"GlobalSection(SolutionConfigurationPlatforms) = preSolution" << Endl;
	os << IncreaseIndent;
	for (std::set< std::wstring >::const_iterator i = availableConfigurations.begin(); i != availableConfigurations.end(); ++i)
		os << *i << L"|" << platform << L" = " << *i << L"|" << platform << L"" << Endl;
	os << DecreaseIndent;
	os << L"EndGlobalSection" << Endl;

	os << L"GlobalSection(ProjectConfigurationPlatforms) = postSolution" << Endl;
	os << IncreaseIndent;

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Project* project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		const RefArray< Configuration >& configurations = project->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			const Configuration* configuration = *j;
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".ActiveCfg = " << configuration->getName() << L"|" << platform << L"" << Endl;
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".Build.0 = " << configuration->getName() << L"|" << platform << L"" << Endl;
		}
	}

	if (m_includeExternal)
	{
		for (RefSet< Solution >::const_iterator i = externalSolutions.begin(); i != externalSolutions.end(); ++i)
		{
			Solution* solution = *i;

			RefArray< Project > externalProjects = solution->getProjects();
			externalProjects.sort(ProjectNamePredicate());

			for (RefArray< Project >::const_iterator j = externalProjects.begin(); j != externalProjects.end(); ++j)
			{
				Project* project = *j;

				// Skip disabled projects.
				if (!project->getEnable())
					continue;

				const RefArray< Configuration >& configurations = project->getConfigurations();
				for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
				{
					const Configuration* configuration = *j;
					os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".ActiveCfg = " << configuration->getName() << L"|" << platform << L"" << Endl;
					os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".Build.0 = " << configuration->getName() << L"|" << platform << L"" << Endl;
				}
			}
		}
	}

	os << DecreaseIndent;
	os << L"EndGlobalSection" << Endl;

	os << DecreaseIndent;
	os << IncreaseIndent;
	
	os << L"GlobalSection(SolutionProperties) = preSolution" << Endl;
	os << IncreaseIndent;
	os << L"HideSolutionNode = FALSE" << Endl;
	os << DecreaseIndent;
	os << L"EndGlobalSection" << Endl;

	if (m_includeExternal && !externalSolutions.empty())
	{
		os << L"GlobalSection(NestedProjects) = preSolution" << Endl;
		os << IncreaseIndent;

		for (RefSet< Solution >::const_iterator i = externalSolutions.begin(); i != externalSolutions.end(); ++i)
		{
			Solution* solution = *i;

			RefArray< Project > externalProjects = solution->getProjects();
			externalProjects.sort(ProjectNamePredicate());

			for (RefArray< Project >::const_iterator j = externalProjects.begin(); j != externalProjects.end(); ++j)
			{
				Project* project = *j;

				// Skip disabled projects.
				if (!project->getEnable())
					continue;

				os << projectGuids[project] << L" = {3E0BD3A9-E831-4928-9EAA-B07D1ED32784}" << Endl;
			}
		}

		os << DecreaseIndent;
		os << L"EndGlobalSection" << Endl;
	}

	os << DecreaseIndent;
	os << L"EndGlobal" << Endl;

	os.close();

	return true;
}

void SolutionBuilderMsvc::showOptions() const
{
	traktor::log::info << L"\t-p=[platform],-msvc-platform=[platform]	(Platform description file)" << Endl;
}

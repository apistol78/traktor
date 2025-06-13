/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Msvc/SolutionBuilderMsvc.h"

#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/String.h"
#include "Core/RefSet.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcSettings.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Utilities.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

// Forced references
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXBuildTool.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXClCompileBuildTool.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXCustomBuildTool.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXDefinition.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImport.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImportGroup.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXProj.h"

namespace traktor::sb
{
namespace
{

struct ProjectNamePredicate
{
	bool operator()(const Project* p0, const Project* p1) const
	{
		return compareIgnoreCase(p0->getName(), p1->getName()) < 0;
	}
};

bool collectExternalSolutions(
	const SolutionBuilderMsvcSettings* settings,
	GeneratorContext& context,
	Project* project,
	std::map< const Project*, std::wstring >& outProjectGuids,
	RefSet< Solution >& outExternalSolutions)
{
	for (auto dependency : project->getDependencies())
	{
		const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency);
		if (!externalDependency)
			continue;

		Solution* externalSolution = externalDependency->getSolution();
		T_ASSERT(externalSolution);

		for (auto externalProject : externalSolution->getProjects())
		{
			if (!externalProject->getEnable())
				continue;
			if (outProjectGuids.find(externalProject) != outProjectGuids.end())
				continue;

			std::wstring solutionPath, projectPath, projectFileName, projectGuid;
			if (!settings->getProject()->getInformation(
					context,
					externalSolution,
					externalProject,
					solutionPath,
					projectPath,
					projectFileName,
					projectGuid))
				return false;

			outProjectGuids[externalProject] = projectGuid;

			if (!collectExternalSolutions(settings, context, externalProject, outProjectGuids, outExternalSolutions))
				return false;
		}

		outExternalSolutions.insert(externalSolution);
	}
	return true;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionBuilderMsvc", SolutionBuilderMsvc, SolutionBuilder)

bool SolutionBuilderMsvc::create(const CommandLine& cmdLine)
{
	Ref< SolutionBuilderMsvcSettings > settings = new SolutionBuilderMsvcSettings();

	if (cmdLine.hasOption('p', L"msvc-platform"))
	{
		std::wstring platform = cmdLine.getOption('p', L"msvc-platform").getString();

		log::info << L"Loading settings \"" << platform << L"\"..." << Endl;

		Ref< IStream > file = FileSystem::getInstance().open(platform, File::FmRead);
		if (!file)
		{
			log::error << L"Unable to open platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		settings = xml::XmlDeserializer(file).readObject< SolutionBuilderMsvcSettings >();
		if (!settings)
		{
			log::error << L"Unable to read platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		file->close();
	}

	if (cmdLine.hasOption('w', L"msvc-write-platform"))
	{
		const std::wstring& outputFileName = cmdLine.getOption('w', L"msvc-write-platform").getString();
		if (outputFileName.empty())
		{
			log::error << L"No output filename specified." << Endl;
			return false;
		}

		log::info << L"Writing settings \"" << outputFileName << L"\"..." << Endl;

		Ref< IStream > file = FileSystem::getInstance().open(outputFileName, File::FmWrite);
		if (!file)
		{
			log::error << L"Unable to create output file \"" << outputFileName << L"\"" << Endl;
			return false;
		}

		xml::XmlSerializer(file).writeObject(settings);

		file->close();
	}

	if (cmdLine.hasOption('i', L"msvc-include-external"))
		m_includeExternal = true;

	m_settings = settings;
	return true;
}

bool SolutionBuilderMsvc::generate(const Solution* solution, const Path& solutionPathName)
{
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXProj)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXBuildTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXClCompileBuildTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXCustomBuildTool)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXDefinition)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXImport)
	T_FORCE_LINK_REF(SolutionBuilderMsvcVCXImportGroup)

	const std::wstring solutionFileName = solution->getRootPath() + L"/" + solution->getName() + L".sln";

	// Setup generator context.
	GeneratorContext context(solutionPathName, m_includeExternal);
	context.set(L"SOLUTION_NAME", solution->getName());
	context.set(L"SOLUTION_ROOTPATH", solution->getRootPath());
	context.set(L"SOLUTION_FILENAME", solutionFileName);

	// Create root path.
	if (!FileSystem::getInstance().makeAllDirectories(solution->getRootPath()))
		return false;

	// Generate projects.
	std::map< const Project*, std::wstring > projectGuids;
	RefSet< Solution > externalSolutions;

	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring solutionPath, projectPath, projectFileName, projectGuid;
		if (!m_settings->getProject()->getInformation(
				context,
				solution,
				project,
				solutionPath,
				projectPath,
				projectFileName,
				projectGuid))
			return false;

		if (!m_settings->getProject()->generate(
				context,
				solution,
				project))
			return false;

		// Collect external projects which we should include in generated solution.
		if (m_includeExternal)
		{
			if (!collectExternalSolutions(m_settings, context, project, projectGuids, externalSolutions))
				return false;
		}

		projectGuids[project] = projectGuid;
	}

	// Generate solution.
	std::wstring solutionGuid = context.generateGUID(solutionFileName);

	AlignedVector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new AnsiEncoding());

	os << L"Microsoft Visual Studio Solution File, Format Version " << m_settings->getSLNVersion() << Endl;
	os << L"# Visual Studio " << m_settings->getVSVersion() << Endl;

	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring solutionPath, projectPath, projectFileName, projectGuid;
		if (!m_settings->getProject()->getInformation(
				context,
				solution,
				project,
				solutionPath,
				projectPath,
				projectFileName,
				projectGuid))
			return false;

		std::wstring projectExtension = Path(projectFileName).getExtension();
		os << L"Project(\"" << solutionGuid << L"\") = \"" << project->getName() << L"\", \"" << project->getName() << L"\\" << project->getName() << L"." << projectExtension << L"\", \"" << /*projectGuids[project]*/ projectGuid << L"\"" << Endl;

		// Add local, or first order external if enabled, dependencies.
		const RefArray< Dependency >& dependencies = project->getDependencies();
		if (!dependencies.empty())
		{
			os << IncreaseIndent;
			os << L"ProjectSection(ProjectDependencies) = postProject" << Endl;

			for (auto dependency : dependencies)
			{
				if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
				{
					Ref< const Project > dependentProject = projectDependency->getProject();
					if (!dependentProject->getEnable())
					{
						log::warning << L"Trying to add disabled dependency (\"" << dependentProject->getName() << L"\") to project \"" << project->getName() << L"\"; dependency skipped." << Endl;
						continue;
					}

					os << IncreaseIndent;
					os << projectGuids[dependentProject] << L" = " << projectGuids[dependentProject] << Endl;
					os << DecreaseIndent;
				}
				else if (m_includeExternal)
				{
					if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
					{
						Ref< const Project > dependentProject = externalDependency->getProject();
						if (!dependentProject->getEnable())
						{
							log::warning << L"Trying to add disabled dependency (\"" << dependentProject->getName() << L"\") to project \"" << project->getName() << L"\"; dependency skipped." << Endl;
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
		for (auto externalSolution : externalSolutions)
		{
			std::wstring externalSolutionId = context.generateGUID(externalSolution->getName());

			os << L"Project(\"{2150E333-8FDC-42A3-9474-1A3956D46DE8}\") = \"" << externalSolution->getName() << L"\", \"" << externalSolution->getName() << L"\", \"" << externalSolutionId << L"\"" << Endl;
			os << L"EndProject" << Endl;

			RefArray< Project > externalProjects = externalSolution->getProjects();
			externalProjects.sort(ProjectNamePredicate());
			for (auto externalProject : externalProjects)
			{
				if (!externalProject->getEnable())
					continue;

				std::wstring solutionPath, projectPath, projectFileName, projectGuid;
				if (!m_settings->getProject()->getInformation(
						context,
						externalSolution,
						externalProject,
						solutionPath,
						projectPath,
						projectFileName,
						projectGuid))
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

					for (auto dependency : dependencies)
					{
						if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
						{
							Ref< const Project > dependentProject = projectDependency->getProject();
							if (!dependentProject->getEnable())
							{
								log::warning << L"Trying to add disabled dependency to project \"" << externalProject->getName() << L"\"; dependency skipped" << Endl;
								continue;
							}

							os << IncreaseIndent;
							os << projectGuids[dependentProject] << L" = " << projectGuids[dependentProject] << Endl;
							os << DecreaseIndent;
						}
						else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
						{
							Ref< const Project > dependentProject = externalDependency->getProject();
							if (!dependentProject)
								continue;

							if (!dependentProject->getEnable())
							{
								log::warning << L"Trying to add disabled dependency to project \"" << externalProject->getName() << L"\"; dependency skipped" << Endl;
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
	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		for (auto configuration : project->getConfigurations())
			availableConfigurations.insert(configuration->getName());
	}

	os << L"GlobalSection(SolutionConfigurationPlatforms) = preSolution" << Endl;
	os << IncreaseIndent;
	for (std::set< std::wstring >::const_iterator i = availableConfigurations.begin(); i != availableConfigurations.end(); ++i)
		os << *i << L"|" << platform << L" = " << *i << L"|" << platform << L"" << Endl;
	os << DecreaseIndent;
	os << L"EndGlobalSection" << Endl;

	os << L"GlobalSection(ProjectConfigurationPlatforms) = postSolution" << Endl;
	os << IncreaseIndent;

	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		for (auto configuration : project->getConfigurations())
		{
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".ActiveCfg = " << configuration->getName() << L"|" << platform << L"" << Endl;
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".Build.0 = " << configuration->getName() << L"|" << platform << L"" << Endl;
		}
	}

	if (m_includeExternal)
	{
		for (auto solution : externalSolutions)
		{
			RefArray< Project > externalProjects = solution->getProjects();
			externalProjects.sort(ProjectNamePredicate());
			for (auto project : externalProjects)
			{
				// Skip disabled projects.
				if (!project->getEnable())
					continue;

				for (auto configuration : project->getConfigurations())
				{
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

		for (auto externalSolution : externalSolutions)
		{
			std::wstring externalSolutionId = context.generateGUID(externalSolution->getName());

			RefArray< Project > externalProjects = externalSolution->getProjects();
			externalProjects.sort(ProjectNamePredicate());
			for (auto project : externalProjects)
			{
				// Skip disabled projects.
				if (!project->getEnable())
					continue;

				os << projectGuids[project] << L" = " << externalSolutionId << Endl;
			}
		}

		os << DecreaseIndent;
		os << L"EndGlobalSection" << Endl;
	}

	os << DecreaseIndent;
	os << L"EndGlobal" << Endl;

	os.close();

	if (!writeFileIfMismatch(solutionFileName, buffer))
		return false;

	return true;
}

void SolutionBuilderMsvc::showOptions() const
{
	log::info << L"\t-p=[platform],--msvc-platform=[platform]			(Platform description file)" << Endl;
	log::info << L"\t-w=[filename],--msvc-write-platform=[filename]		(Write platform description file)" << Endl;
}

}

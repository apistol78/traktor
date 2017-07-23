/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ScriptProcessor.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Eclipse/SolutionBuilderEclipse.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderEclipse", SolutionBuilderEclipse, SolutionBuilder)

SolutionBuilderEclipse::SolutionBuilderEclipse()
{
}

bool SolutionBuilderEclipse::create(const traktor::CommandLine& cmdLine)
{
	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create())
		return false;

	if (cmdLine.hasOption('p', L"project"))
		m_projectFile = cmdLine.getOption('p', L"project").getString();

	if (cmdLine.hasOption('c', L"cproject"))
		m_cprojectFile = cmdLine.getOption('c', L"cproject").getString();

	return true;
}

bool SolutionBuilderEclipse::generate(Solution* solution)
{
	// Create root path.
	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath()))
	{
		traktor::log::error << L"Unable to make directory \"" << solution->getRootPath() << L"\"" << Endl;
		return false;
	}

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const Project* project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		const RefArray< ProjectItem >& items = project->getItems();

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();
		std::wstring projectFileName = projectPath + L"/.project";
		std::wstring cprojectFileName = projectPath + L"/.cproject";

		if (!FileSystem::getInstance().makeDirectory(projectPath))
		{
			traktor::log::error << L"Unable to make directory \"" << projectPath << L"\"" << Endl;
			return false;
		}

		// Generate .project
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, m_projectFile, projectOut))
			{
				traktor::log::error << L"Unable to generate project \"" << projectFileName << L"\" using \"" << m_projectFile << L"\"" << Endl;
				return false;
			}

			Ref< IStream > file = FileSystem::getInstance().open(
				projectFileName,
				traktor::File::FmWrite
			);
			if (!file)
			{
				traktor::log::error << L"Unable to create project \"" << projectFileName << L"\"" << Endl;
				return false;
			}

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}

		// Generate .cproject
		{
			std::wstring cprojectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, m_cprojectFile, cprojectOut))
			{
				traktor::log::error << L"Unable to generate cproject \"" << cprojectFileName << L"\" using template \"" << m_cprojectFile << L"\"" << Endl;
				return false;
			}

			Ref< IStream > file = FileSystem::getInstance().open(
				cprojectFileName,
				traktor::File::FmWrite
			);
			if (!file)
			{
				traktor::log::error << L"Unable to create cproject \"" << cprojectFileName << L"\"" << Endl;
				return false;
			}

			FileOutputStream(file, new Utf8Encoding()) << cprojectOut;

			file->close();
		}
	}

	return true;
}

void SolutionBuilderEclipse::showOptions() const
{
	traktor::log::info << L"\t-p,--project=[project template]	(project template file)" << Endl;
	traktor::log::info << L"\t-c,--cproject=[cproject template]	(cproject template file)" << Endl;
}

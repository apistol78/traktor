/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/FBuild/SolutionBuilderFBuild.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderFBuild", SolutionBuilderFBuild, SolutionBuilder)

SolutionBuilderFBuild::SolutionBuilderFBuild()
{
}

SolutionBuilderFBuild::~SolutionBuilderFBuild()
{
	safeDestroy(m_scriptProcessor);
}

bool SolutionBuilderFBuild::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption(L"fastbuild-solution-template"))
		m_solutionTemplate = cmdLine.getOption(L"fastbuild-solution-template").getString();
	if (cmdLine.hasOption(L"fastbuild-project-template"))
		m_projectTemplate = cmdLine.getOption(L"fastbuild-project-template").getString();

	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create())
		return false;

	return true;
}

bool SolutionBuilderFBuild::generate(Solution* solution)
{
	// Create root path.
	if (!FileSystem::getInstance().makeAllDirectories(solution->getRootPath()))
	{
		log::error << L"Unable to create output directory \"" << solution->getRootPath() << L"\"." << Endl;
		return false;
	}

	log::info << L"Generating FASTBuild projects..." << Endl;

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const Project* project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();

		if (!FileSystem::getInstance().makeDirectory(projectPath))
		{
			log::error << L"Unable to create project output directory \"" << projectPath << L"\"." << Endl;
			return false;
		}

		// Generate project
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, m_projectTemplate, projectOut))
			{
				log::error << L"Script processor failed using project template \"" << m_projectTemplate << L"\"." << Endl;
				return false;
			}

			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/fbuild.bff",
				File::FmWrite
			);
			if (!file)
			{
				log::error << L"Unable to create project file \"" << projectPath << L"/" << project->getName() << L".bff" << L"\"." << Endl;
				return false;
			}

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}
	}

	log::info << L"Generating FASTBuild solution..." << Endl;

	// Generate build file.
	{
		std::wstring solutionOut;
		if (!m_scriptProcessor->generateFromFile(solution, 0, solution->getRootPath(), m_solutionTemplate, solutionOut))
		{
			log::error << L"Script processor failed using solution template \"" << m_solutionTemplate << L"\"." << Endl;
			return false;
		}

		Ref< IStream > file = FileSystem::getInstance().open(
			solution->getRootPath() + L"/" + solution->getName() + L".bff",
			File::FmWrite
		);
		if (!file)
		{
			log::error << L"Unable to create solution file \"" << solution->getRootPath() << L"/" + solution->getName() << L".bff" << L"\"." << Endl;
			return false;
		}

		FileOutputStream(file, new Utf8Encoding()) << solutionOut;

		file->close();
	}

	return true;
}

void SolutionBuilderFBuild::showOptions() const
{
	log::info << L"\t-fastbuild-solution-template=[template file]" << Endl;
	log::info << L"\t-fastbuild-project-template=[template file]" << Endl;
}

	}
}

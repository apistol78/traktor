/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionBuilderFBuild", SolutionBuilderFBuild, SolutionBuilder)

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
	if (!m_scriptProcessor->create(cmdLine))
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

	if (!m_scriptProcessor->prepare(m_projectTemplate))
	{
		log::error << L"Script processor failed using project template \"" << m_projectTemplate << L"\"." << Endl;
		return false;
	}

	for (auto project : solution->getProjects())
	{
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
			if (!m_scriptProcessor->generate(solution, project, L"", projectPath, projectOut))
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

	if (!m_scriptProcessor->prepare(m_solutionTemplate))
	{
		log::error << L"Script processor failed using solution template \"" << m_solutionTemplate << L"\"." << Endl;
		return false;
	}

	// Generate build file.
	{
		std::wstring solutionOut;
		if (!m_scriptProcessor->generate(solution, nullptr, L"", solution->getRootPath(), solutionOut))
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

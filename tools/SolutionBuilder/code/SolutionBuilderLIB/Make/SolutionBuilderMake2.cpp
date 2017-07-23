/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include <Core/Misc/SafeDestroy.h>
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ScriptProcessor.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Make/SolutionBuilderMake2.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMake2", SolutionBuilderMake2, SolutionBuilder)

SolutionBuilderMake2::SolutionBuilderMake2()
{
}

SolutionBuilderMake2::~SolutionBuilderMake2()
{
	safeDestroy(m_scriptProcessor);
}

bool SolutionBuilderMake2::create(const traktor::CommandLine& cmdLine)
{
	if (cmdLine.hasOption('s', L"make-solution-template"))
		m_solutionTemplate = cmdLine.getOption('s', L"make-solution-template").getString();
	if (cmdLine.hasOption('p', L"make-project-template"))
		m_projectTemplate = cmdLine.getOption('p', L"make-project-template").getString();

	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create())
		return false;

	return true;
}

bool SolutionBuilderMake2::generate(Solution* solution)
{
	// Create root path.
	if (!FileSystem::getInstance().makeAllDirectories(solution->getRootPath()))
		return false;

	log::info << L"Generating project makefiles..." << Endl;

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const Project* project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();

		if (!FileSystem::getInstance().makeDirectory(projectPath))
			return false;

		// Generate project makefile.
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, m_projectTemplate, projectOut))
				return false;

			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/makefile",
				traktor::File::FmWrite
			);
			if (!file)
				return false;

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}
	}

	log::info << L"Generating solution makefile..." << Endl;

	// Generate solution makefile.
	{
		std::wstring cprojectOut;
		if (!m_scriptProcessor->generateFromFile(solution, 0, solution->getRootPath(), m_solutionTemplate, cprojectOut))
			return false;

		Ref< IStream > file = FileSystem::getInstance().open(
			solution->getRootPath() + L"/makefile",
			traktor::File::FmWrite
		);
		if (!file)
			return false;

		FileOutputStream(file, new Utf8Encoding()) << cprojectOut;

		file->close();
	}

	return true;
}

void SolutionBuilderMake2::showOptions() const
{
	traktor::log::info << L"\t-s,-make-solution-template=[solution template file]" << Endl;
	traktor::log::info << L"\t-p,-make-project-template=[project template file]" << Endl;
}

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
#include "Core/Timer/Timer.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Make/SolutionBuilderMake2.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMake2", SolutionBuilderMake2, SolutionBuilder)

SolutionBuilderMake2::SolutionBuilderMake2()
{
}

SolutionBuilderMake2::~SolutionBuilderMake2()
{
	safeDestroy(m_scriptProcessor);
}

bool SolutionBuilderMake2::create(const CommandLine& cmdLine)
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
	Timer timer;

	// Create root path.
	if (!FileSystem::getInstance().makeAllDirectories(solution->getRootPath()))
		return false;

	log::info << L"Generating project makefiles..." << Endl;
	log::info << IncreaseIndent;

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const Project* project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();
		log::info << projectPath + L"/makefile";

		double timeStart = timer.getElapsedTime();

		if (!FileSystem::getInstance().makeDirectory(projectPath))
			return false;

		// Generate project makefile.
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, m_projectTemplate, projectOut))
				return false;

			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/makefile",
				File::FmWrite
			);
			if (!file)
				return false;

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}

		double timeEnd = timer.getElapsedTime();
		log::info << L" (" << int32_t((timeEnd - timeStart) + 0.5) << L" s)" << Endl;
	}

	log::info << DecreaseIndent;
	log::info << Endl;

	log::info << L"Generating solution makefile..." << Endl;
	log::info << IncreaseIndent;

	// Generate solution makefile.
	{
		std::wstring cprojectOut;
		if (!m_scriptProcessor->generateFromFile(solution, 0, solution->getRootPath(), m_solutionTemplate, cprojectOut))
			return false;

		std::wstring solutionPath = solution->getRootPath() + L"/" + solution->getName() + L".mak";
		log::info << solutionPath;

		double timeStart = timer.getElapsedTime();

		Ref< IStream > file = FileSystem::getInstance().open(
			solutionPath,
			File::FmWrite
		);
		if (!file)
			return false;

		FileOutputStream(file, new Utf8Encoding()) << cprojectOut;

		file->close();

		double timeEnd = timer.getElapsedTime();
		log::info << L" (" << int32_t((timeEnd - timeStart) + 0.5) << L" s)" << Endl;
	}

	log::info << DecreaseIndent;
	log::info << Endl;

	return true;
}

void SolutionBuilderMake2::showOptions() const
{
	log::info << L"\t-s,-make-solution-template=[solution template file]" << Endl;
	log::info << L"\t-p,-make-project-template=[project template file]" << Endl;
}

	}
}

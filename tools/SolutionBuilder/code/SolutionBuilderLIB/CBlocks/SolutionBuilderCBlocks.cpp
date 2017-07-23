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
#include "SolutionBuilderLIB/CBlocks/SolutionBuilderCBlocks.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderCBlocks", SolutionBuilderCBlocks, SolutionBuilder)

SolutionBuilderCBlocks::SolutionBuilderCBlocks()
{
}

SolutionBuilderCBlocks::~SolutionBuilderCBlocks()
{
	safeDestroy(m_scriptProcessor);
}

bool SolutionBuilderCBlocks::create(const traktor::CommandLine& cmdLine)
{
	if (cmdLine.hasOption('w', L"cblocks-workspace-template"))
		m_workspaceTemplate = cmdLine.getOption('w', L"cblocks-workspace-template").getString();
	if (cmdLine.hasOption('p', L"cblocks-project-template"))
		m_projectTemplate = cmdLine.getOption('p', L"cblocks-project-template").getString();

	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create())
		return false;

	return true;
}

bool SolutionBuilderCBlocks::generate(Solution* solution)
{
	// Create root path.
	if (!FileSystem::getInstance().makeAllDirectories(solution->getRootPath()))
		return false;

	log::info << L"Generating projects..." << Endl;

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

		// Generate project
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, m_projectTemplate, projectOut))
				return false;

			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/" + project->getName() + L".cbp",
				traktor::File::FmWrite
			);
			if (!file)
				return false;

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}
	}

	log::info << L"Generating workspace..." << Endl;

	// Generate workspace
	{
		std::wstring cprojectOut;
		if (!m_scriptProcessor->generateFromFile(solution, 0, solution->getRootPath(), m_workspaceTemplate, cprojectOut))
			return false;

		Ref< IStream > file = FileSystem::getInstance().open(
			solution->getRootPath() + L"/" + solution->getName() + L".workspace",
			traktor::File::FmWrite
		);
		if (!file)
			return false;

		FileOutputStream(file, new Utf8Encoding()) << cprojectOut;

		file->close();
	}

	return true;
}

void SolutionBuilderCBlocks::showOptions() const
{
	traktor::log::info << L"\t-w,-cblocks-workspace-template=[workspace template file]" << Endl;
	traktor::log::info << L"\t-p,-cblocks-project-template=[project template file]" << Endl;
}

#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/CBlocks/SolutionBuilderCBlocks.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderCBlocks", SolutionBuilderCBlocks, SolutionBuilder)

SolutionBuilderCBlocks::SolutionBuilderCBlocks()
{
}

SolutionBuilderCBlocks::~SolutionBuilderCBlocks()
{
	safeDestroy(m_scriptProcessor);
}

bool SolutionBuilderCBlocks::create(const CommandLine& cmdLine)
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

	if (!m_scriptProcessor->prepare(m_projectTemplate))
		return false;

	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();

		if (!FileSystem::getInstance().makeDirectory(projectPath))
			return false;

		// Generate project
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generate(solution, project, L"", projectPath, projectOut))
				return false;

			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/" + project->getName() + L".cbp",
				File::FmWrite
			);
			if (!file)
				return false;

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}
	}

	log::info << L"Generating workspace..." << Endl;

	if (!m_scriptProcessor->prepare(m_workspaceTemplate))
		return false;

	// Generate workspace
	{
		std::wstring cprojectOut;
		if (!m_scriptProcessor->generate(solution, nullptr, L"", solution->getRootPath(), cprojectOut))
			return false;

		Ref< IStream > file = FileSystem::getInstance().open(
			solution->getRootPath() + L"/" + solution->getName() + L".workspace",
			File::FmWrite
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
	log::info << L"\t-w,-cblocks-workspace-template=[workspace template file]" << Endl;
	log::info << L"\t-p,-cblocks-project-template=[project template file]" << Endl;
}

	}
}

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
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, L"$(TRAKTOR_HOME)/bin/cblocks-linux-gcc-project.sb", projectOut))
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
		if (!m_scriptProcessor->generateFromFile(solution, 0, solution->getRootPath(), L"$(TRAKTOR_HOME)/bin/cblocks-linux-gcc-workspace.sb", cprojectOut))
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
}

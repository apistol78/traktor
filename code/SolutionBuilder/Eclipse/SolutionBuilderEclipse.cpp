#include <algorithm>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/File.h"
#include "SolutionBuilder/Filter.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Eclipse/SolutionBuilderEclipse.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderEclipse", SolutionBuilderEclipse, SolutionBuilder)

SolutionBuilderEclipse::SolutionBuilderEclipse()
{
}

bool SolutionBuilderEclipse::create(const CommandLine& cmdLine)
{
	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create(cmdLine))
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
		log::error << L"Unable to make directory \"" << solution->getRootPath() << L"\"" << Endl;
		return false;
	}

	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();
		std::wstring projectFileName = projectPath + L"/.project";
		std::wstring cprojectFileName = projectPath + L"/.cproject";

		if (!FileSystem::getInstance().makeDirectory(projectPath))
		{
			log::error << L"Unable to make directory \"" << projectPath << L"\"" << Endl;
			return false;
		}

		// Generate .project
		{
			if (!m_scriptProcessor->prepare(m_projectFile))
				return false;

			std::wstring projectOut;
			if (!m_scriptProcessor->generate(solution, project, L"", projectPath, projectOut))
			{
				log::error << L"Unable to generate project \"" << projectFileName << L"\" using \"" << m_projectFile << L"\"" << Endl;
				return false;
			}

			Ref< IStream > file = FileSystem::getInstance().open(
				projectFileName,
				traktor::File::FmWrite
			);
			if (!file)
			{
				log::error << L"Unable to create project \"" << projectFileName << L"\"" << Endl;
				return false;
			}

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}

		// Generate .cproject
		{
			if (!m_scriptProcessor->prepare(m_cprojectFile))
				return false;

			std::wstring cprojectOut;
			if (!m_scriptProcessor->generate(solution, project, L"", projectPath, cprojectOut))
			{
				log::error << L"Unable to generate cproject \"" << cprojectFileName << L"\" using template \"" << m_cprojectFile << L"\"" << Endl;
				return false;
			}

			Ref< IStream > file = FileSystem::getInstance().open(
				cprojectFileName,
				traktor::File::FmWrite
			);
			if (!file)
			{
				log::error << L"Unable to create cproject \"" << cprojectFileName << L"\"" << Endl;
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
	log::info << L"\t-p,--project=[project template]	(project template file)" << Endl;
	log::info << L"\t-c,--cproject=[cproject template]	(cproject template file)" << Endl;
}

	}
}

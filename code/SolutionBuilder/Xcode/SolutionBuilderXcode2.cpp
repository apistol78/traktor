#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Xcode/SolutionBuilderXcode2.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderXcode2", SolutionBuilderXcode2, SolutionBuilder)

SolutionBuilderXcode2::~SolutionBuilderXcode2()
{
	safeDestroy(m_scriptProcessor);
}

bool SolutionBuilderXcode2::create(const CommandLine& cmdLine)
{
	if (!cmdLine.hasOption(L"xcode-project-template") || !cmdLine.hasOption(L"xcode-workspace-template"))
		return false;

	m_projectTemplate = cmdLine.getOption(L"xcode-project-template").getString();
	m_workspaceTemplate = cmdLine.getOption(L"xcode-workspace-template").getString();

	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create())
		return false;

	return true;
}

bool SolutionBuilderXcode2::generate(Solution* solution)
{
	log::info << L"Generating Xcode projects..." << Endl;

	if (!m_scriptProcessor->prepare(m_projectTemplate))
	{
		log::error << L"Script processor failed using template \"" << m_projectTemplate << L"\"." << Endl;
		return false;
	}

	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		log::info << L"Generating " << project->getName() << L"..." << Endl;

		std::wstring projectPath = solution->getRootPath() + L"/wip/" + project->getName() + L".xcodeproj";

		if (!FileSystem::getInstance().makeAllDirectories(projectPath))
		{
			log::error << L"Unable to create project output directory \"" << projectPath << L"\"." << Endl;
			return false;
		}

		// Generate project
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generate(solution, project, projectPath, projectOut))
			{
				log::error << L"Script processor failed using project template \"" << m_projectTemplate << L"\"." << Endl;
				return false;
			}

			Ref< IStream > file = FileSystem::getInstance().open(
				projectPath + L"/project.pbxproj",
				File::FmWrite
			);
			if (!file)
			{
				//log::error << L"Unable to create project file \"" << projectPath << L"/" << project->getName() << L".ninja" << L"\"." << Endl;
				return false;
			}

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();
		}
	}

	log::info << L"Generating Xcode workspace..." << Endl;

	if (!m_scriptProcessor->prepare(m_workspaceTemplate))
	{
		log::error << L"Script processor failed using solution template \"" << m_workspaceTemplate << L"\"." << Endl;
		return false;
	}

	{
		std::wstring workspacePath = solution->getRootPath() + L"/wip/" + solution->getName() + L".xcworkspace";

		if (!FileSystem::getInstance().makeAllDirectories(workspacePath))
		{
			log::error << L"Unable to create project output directory \"" << workspacePath << L"\"." << Endl;
			return false;
		}

		std::wstring solutionOut;
		if (!m_scriptProcessor->generate(solution, nullptr, solution->getRootPath(), solutionOut))
		{
			log::error << L"Script processor failed using template \"" << m_workspaceTemplate << L"\"." << Endl;
			return false;
		}

		Ref< IStream > file = FileSystem::getInstance().open(
			workspacePath + L"/contents.xcworkspacedata",
			File::FmWrite
		);
		if (!file)
		{
			//log::error << L"Unable to create project file \"" << outputPath << L"/project.pbxproj\"." << Endl;
			return false;
		}

		FileOutputStream(file, new Utf8Encoding()) << solutionOut;

		file->close();
	}

	log::info << L"Done" << Endl;
	return true;
}

void SolutionBuilderXcode2::showOptions() const
{
	//log::info << L"\t-ninja-solution-template=[template file]" << Endl;
	//log::info << L"\t-ninja-project-template=[template file]" << Endl;
}

	}
}

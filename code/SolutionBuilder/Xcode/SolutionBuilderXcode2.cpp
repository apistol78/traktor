#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "SolutionBuilder/Configuration.h"
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
	if (!cmdLine.hasOption(L"xcode-project-template") || !cmdLine.hasOption(L"xcode-workspace-template") || !cmdLine.hasOption(L"xcode-workspace-scheme-template"))
		return false;

	m_projectTemplate = cmdLine.getOption(L"xcode-project-template").getString();
	m_workspaceTemplate = cmdLine.getOption(L"xcode-workspace-template").getString();
	m_workspaceSchemeTemplate = cmdLine.getOption(L"xcode-workspace-scheme-template").getString();

	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create(cmdLine))
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

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName() + L".xcodeproj";
		if (!FileSystem::getInstance().makeAllDirectories(projectPath))
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
				projectPath + L"/project.pbxproj",
				File::FmWrite
			);
			if (!file)
			{
				log::error << L"Unable to create project file \"" << projectPath << L"/project.pbxproj" << L"\"." << Endl;
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
		std::wstring workspacePath = solution->getRootPath() + L"/" + solution->getName() + L".xcworkspace";
		if (!FileSystem::getInstance().makeAllDirectories(workspacePath))
		{
			log::error << L"Unable to create project output directory \"" << workspacePath << L"\"." << Endl;
			return false;
		}

		std::wstring solutionOut;
		if (!m_scriptProcessor->generate(solution, nullptr, L"", solution->getRootPath(), solutionOut))
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
			log::error << L"Unable to create workspace file \"" << workspacePath << L"/contents.xcworkspacedata\"." << Endl;
			return false;
		}
		FileOutputStream(file, new Utf8Encoding()) << solutionOut;
		file->close();
	}

	log::info << L"Generating Xcode workspace scheme..." << Endl;

	if (!m_scriptProcessor->prepare(m_workspaceSchemeTemplate))
	{
		log::error << L"Script processor failed using solution template \"" << m_workspaceSchemeTemplate << L"\"." << Endl;
		return false;
	}

	// Get all unique, and enabled, configuration names.
	SmallSet< std::wstring > configurations;
	for (auto project : solution->getProjects())
	{
		if (project->getEnable())
		{
			for (auto configuration : project->getConfigurations())
				configurations.insert(configuration->getName());
		}
	}

	std::wstring schemePath = solution->getRootPath() + L"/" + solution->getName() + L".xcworkspace/xcshareddata/xcschemes";
	if (!FileSystem::getInstance().makeAllDirectories(schemePath))
	{
		log::error << L"Unable to create project output directory \"" << schemePath << L"\"." << Endl;
		return false;
	}
	
	for (const auto& configuration : configurations)
	{
		std::wstring schemeOut;
		if (!m_scriptProcessor->generate(solution, nullptr, configuration, solution->getRootPath(), schemeOut))
		{
			log::error << L"Script processor failed using template \"" << m_workspaceSchemeTemplate << L"\"." << Endl;
			return false;
		}

		Ref< IStream > file = FileSystem::getInstance().open(
			schemePath + L"/" + configuration + L".xcscheme",
			File::FmWrite
		);
		if (!file)
		{
			log::error << L"Unable to create scheme file \"" << schemePath + L"/" << configuration << L".xcscheme\"." << Endl;
			return false;
		}
		FileOutputStream(file, new Utf8Encoding()) << schemeOut;
		file->close();
	}

	log::info << L"Done" << Endl;
	return true;
}

void SolutionBuilderXcode2::showOptions() const
{
	log::info << L"\t-xcode-project-template=[template file]" << Endl;
	log::info << L"\t-xcode-workspace-template=[template file]" << Endl;
	log::info << L"\t-xcode-workspace-scheme-template=[template file]" << Endl;
}

	}
}

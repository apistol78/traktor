/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/SmallSet.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Core/System/ResolveEnv.h"
#include "Core/Timer/Timer.h"
#include "SolutionBuilder/CBlocks/SolutionBuilderCBlocks.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Dependencies/SolutionBuilderDependencies.h"
#include "SolutionBuilder/Direct/SolutionBuilderDirect.h"
#include "SolutionBuilder/Eclipse/SolutionBuilderEclipse.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/FBuild/SolutionBuilderFBuild.h"
#include "SolutionBuilder/GraphViz/SolutionBuilderGraphViz.h"
#include "SolutionBuilder/Make/SolutionBuilderMake2.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilder/Ninja/SolutionBuilderNinja.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/SolutionLoader.h"
#include "SolutionBuilder/Utilities.h"
#include "SolutionBuilder/Version.h"
#include "SolutionBuilder/Xcode/SolutionBuilderXcode2.h"

using namespace traktor;
using namespace traktor::sb;

#define ERROR_UNKNOWN_FORMAT 1
#define ERROR_UNABLE_TO_READ_SOLUTION 2
#define ERROR_UNABLE_TO_RESOLVE_DEPENDENCIES 3
#define ERROR_UNABLE_TO_CREATE_BUILDER 4
#define ERROR_UNABLE_TO_CREATE_SOLUTION 5

/*! Set environment variables of solution into process, external solutions first so solution's own take precedence. */
void applyEnvironmentVariables(const Solution* solution, SmallSet< const Solution* >& visited, bool verbose)
{
	if (!visited.insert(solution))
		return;

	for (auto project : solution->getProjects())
		for (auto dependency : project->getDependencies())
			if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
				applyEnvironmentVariables(externalDependency->getSolution(), visited, verbose);

	for (const auto& variable : solution->getEnvironmentVariables())
	{
		std::wstring value;
		if (!variable.overwrite && OS::getInstance().getEnvironment(variable.name, value))
			continue;

		value = resolveEnv(variable.value, nullptr);
		OS::getInstance().setEnvironment(variable.name, value);

		if (verbose)
			traktor::log::info << L"Environment " << variable.name << L" = \"" << value << L"\"" << Endl;
	}
}

int main(int argc, const char** argv)
{
	T_FORCE_LINK_REF(ProjectDependency)
	T_FORCE_LINK_REF(ExternalDependency)

	CommandLine cmdLine(argc, argv);
	Ref< SolutionBuilder > builder;

	if (cmdLine.hasOption('f', L"format"))
	{
		std::wstring ide = cmdLine.getOption('f', L"format").getString();
		if (ide == L"cblocks")
			builder = new SolutionBuilderCBlocks();
		else if (ide == L"direct")
			builder = new SolutionBuilderDirect();
		else if (ide == L"eclipse")
			builder = new SolutionBuilderEclipse();
		else if (ide == L"fbuild")
			builder = new SolutionBuilderFBuild();
		else if (ide == L"graphviz")
			builder = new SolutionBuilderGraphViz();
		else if (ide == L"make2")
			builder = new SolutionBuilderMake2();
		else if (ide == L"msvc")
			builder = new SolutionBuilderMsvc();
		else if (ide == L"ninja")
			builder = new SolutionBuilderNinja();
		else if (ide == L"xcode")
			builder = new SolutionBuilderXcode2();
		else if (ide == L"dependencies")
			builder = new SolutionBuilderDependencies();
		else
		{
			traktor::log::error << L"Unknown format \"" << ide << L"\"." << Endl;
			return ERROR_UNKNOWN_FORMAT;
		}
	}
	else
		builder = new SolutionBuilderMsvc();

	if (cmdLine.hasOption('?') || cmdLine.hasOption('h', L"help"))
	{
		traktor::log::info << SB_TITLE << Endl;
		traktor::log::info << L"Usage : " << Path(cmdLine.getFile()).getFileName() << L" -[options] [solution] (include project names)" << Endl;
		traktor::log::info << L"\t-f,--format=[format]  [\"cblocks\", \"direct\", \"eclipse\", \"graphviz\", \"msvc\"*, \"make\", \"make2\", \"ninja\", \"xcode\", \"dependencies\"]" << Endl;
		traktor::log::info << L"\t--rootPath=Path       Override solution root path" << Endl;
		traktor::log::info << L"\t--verbose             Verbose" << Endl;
		traktor::log::info << L"\t--profile             Profile build scripts" << Endl;
		if (builder)
			builder->showOptions();
		return 0;
	}

	// Check if environment is already set, else set to current working directory.
	std::wstring home;
	if (!OS::getInstance().getEnvironment(L"TRAKTOR_HOME", home))
	{
		const Path originalCwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
		const Path executablePath = OS::getInstance().getExecutable().getPathOnly();
		FileSystem::getInstance().setCurrentVolumeAndDirectory(executablePath);

		while (!FileSystem::getInstance().exist(L"LICENSE.txt"))
		{
			const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
			const Path pwd = cwd.getPathOnly();
			if (cwd == pwd)
				break;
			FileSystem::getInstance().setCurrentVolumeAndDirectory(pwd);
		}

		const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
		if (!cwd.empty())
		{
			OS::getInstance().setEnvironment(L"TRAKTOR_HOME", cwd.getPathNameOS());
			FileSystem::getInstance().setCurrentVolumeAndDirectory(originalCwd);
		}
	}

	Timer timer;
	Path solutionPathName;
	Ref< Solution > solution;

	if (cmdLine.getCount() >= 1)
	{
		SolutionLoader solutionLoader;

		solutionPathName = cmdLine.getString(0);

		if (cmdLine.hasOption('v', L"verbose"))
		{
			traktor::log::info << SB_TITLE << Endl;
			traktor::log::info << L"Loading solution \"" << solutionPathName.getPathName() << L"\"..." << Endl;
		}

		solution = solutionLoader.load(cmdLine.getString(0));
		if (!solution)
		{
			traktor::log::error << L"Unable to read solution \"" << solutionPathName.getPathName() << L"\"" << Endl;
			return ERROR_UNABLE_TO_READ_SOLUTION;
		}

		SmallSet< const Solution* > visited;
		applyEnvironmentVariables(solution, visited, cmdLine.hasOption('v', L"verbose"));

		if (cmdLine.hasOption(L"rootPath"))
		{
			const Path rootPath(cmdLine.getOption(L"rootPath").getString());
			solution->setRootPath(rootPath.normalized().getPathName());
		}

		if (cmdLine.hasOption('v', L"verbose"))
			traktor::log::info << L"Using root path \"" << solution->getRootPath() << L"\"" << Endl;

		if (cmdLine.hasOption('v', L"verbose"))
			traktor::log::info << L"Flatten include paths..." << Endl;

		// Direct builder selects projects itself, dependencies of selected projects must stay enabled.
		if (cmdLine.getCount() > 1 && !is_a< SolutionBuilderDirect >(builder))
		{
			for (auto project : solution->getProjects())
			{
				if (!project->getEnable())
					continue;

				bool include = false;
				for (uint32_t i = 1; !include && i < cmdLine.getCount(); ++i)
					if (cmdLine.getString(i) == project->getName())
						include = true;

				project->setEnable(include);
			}
		}

		for (auto project : solution->getProjects())
		{
			SmallMap< std::wstring, SmallSet< std::wstring > > configurationIncludePaths;
			SmallMap< std::wstring, SmallSet< std::wstring > > configurationLibraryPaths;
			SmallSet< const Project* > visited;
			flattenIncludePaths(solutionPathName, project, visited, configurationIncludePaths, configurationLibraryPaths);

			for (auto configuration : project->getConfigurations())
			{
				const auto& includePaths = configurationIncludePaths[configuration->getName()];
				const auto& libraryPaths = configurationLibraryPaths[configuration->getName()];

				if (cmdLine.hasOption('v', L"verbose"))
				{
					traktor::log::info << L"Include paths of \"" << project->getName() << L"\" " << configuration->getName() << Endl;
					for (auto includePath : includePaths)
						traktor::log::info << L"\t" << includePath << Endl;

					traktor::log::info << L"Library paths of \"" << project->getName() << L"\" " << configuration->getName() << Endl;
					for (auto libraryPath : libraryPaths)
						traktor::log::info << L"\t" << libraryPath << Endl;
				}

				configuration->setIncludePaths(std::vector< std::wstring >(
					includePaths.begin(),
					includePaths.end()));

				configuration->setLibraryPaths(std::vector< std::wstring >(
					libraryPaths.begin(),
					libraryPaths.end()));
			}
		}
	}

	if (!builder || !builder->create(cmdLine))
	{
		traktor::log::error << L"Unable to create solution builder." << Endl;
		return ERROR_UNABLE_TO_CREATE_BUILDER;
	}

	if (cmdLine.hasOption('v', L"verbose"))
		traktor::log::info << L"Generating target solution..." << Endl;

	if (solution)
	{
		if (!builder->generate(solution, solutionPathName))
		{
			traktor::log::error << L"Unable to generate target solution." << Endl;
			return ERROR_UNABLE_TO_CREATE_SOLUTION;
		}
	}

	if (cmdLine.hasOption('v', L"verbose"))
		traktor::log::info << L"Finished successfully in " << timer.getElapsedTime() << L" second(s)." << Endl;

	return 0;
}

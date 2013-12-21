#include <Core/Io/Path.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/TString.h>
#include <Core/Timer/Timer.h>
#include "SolutionBuilderLIB/Aggregation.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/SolutionLoader.h"
#include "SolutionBuilderLIB/CBlocks/SolutionBuilderCBlocks.h"
#include "SolutionBuilderLIB/Eclipse/SolutionBuilderEclipse.h"
#include "SolutionBuilderLIB/GraphViz/SolutionBuilderGraphViz.h"
#include "SolutionBuilderLIB/Make/SolutionBuilderMake.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilderLIB/Xcode/SolutionBuilderXcode.h"

using namespace traktor;

#define TITLE L"SolutionBuilder v2.7.6"

#define ERROR_UNKNOWN_FORMAT 1
#define ERROR_UNABLE_TO_READ_SOLUTION 2
#define ERROR_UNABLE_TO_RESOLVE_DEPENDENCIES 3
#define ERROR_UNABLE_TO_CREATE_BUILDER 4
#define ERROR_UNABLE_TO_CREATE_SOLUTION 5

int main(int argc, const char** argv)
{
	T_FORCE_LINK_REF(ProjectDependency)
	T_FORCE_LINK_REF(ExternalDependency)

	CommandLine cmdLine(argc, argv);
	Ref< SolutionBuilder > builder;

	traktor::log::info << TITLE << Endl;

	if (cmdLine.hasOption('f', L"format"))
	{
		std::wstring ide = cmdLine.getOption('f', L"format").getString();
		if (ide == L"cblocks")
			builder = new SolutionBuilderCBlocks();
		else if (ide == L"eclipse")
			builder = new SolutionBuilderEclipse();
		else if (ide == L"graphviz")
			builder = new SolutionBuilderGraphViz();
		else if (ide == L"make")
			builder = new SolutionBuilderMake();
		else if (ide == L"msvc")
			builder = new SolutionBuilderMsvc();
		else if (ide == L"xcode")
			builder = new SolutionBuilderXcode();
		else
		{
			traktor::log::error << L"Unknown format \"" << ide << L"\"" << Endl;
			return ERROR_UNKNOWN_FORMAT;
		}
	}
	else
		builder = new SolutionBuilderMsvc();

	if (cmdLine.hasOption('?') || cmdLine.hasOption('h', L"help") || cmdLine.getCount() <= 0)
	{
		traktor::log::info << L"Usage : " << Path(cmdLine.getFile()).getFileName() << L" -[options] [solution]" << Endl;
		traktor::log::info << L"\t-f,-format=[format]	[\"cblocks\", \"eclipse\", \"graphviz\", \"msvc\"*, \"make\", \"xcode\"]" << Endl;
		traktor::log::info << L"\t-rootPath=Path		Override solution root path" << Endl;
		if (builder)
			builder->showOptions();
		return 0;
	}

	Timer timer;
	timer.start();

	SolutionLoader solutionLoader;

	traktor::log::info << L"Loading solution \"" << cmdLine.getString(0) << L"\"..." << Endl;

	Ref< Solution > solution = solutionLoader.load(cmdLine.getString(0));
	if (!solution)
	{
		traktor::log::error << L"Unable to read solution \"" << cmdLine.getString(0) << L"\"" << Endl;
		return ERROR_UNABLE_TO_READ_SOLUTION;
	}

	if (cmdLine.hasOption(L"rootPath"))
	{
		Path rootPath(cmdLine.getOption(L"rootPath").getString());
		solution->setRootPath(rootPath.normalized().getPathName());
	}

	traktor::log::info << L"Using root path \"" << solution->getRootPath() << L"\"" << Endl;
	traktor::log::info << L"Resolving dependencies..." << Endl;

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (!(*j)->resolve(&solutionLoader))
			{
				traktor::log::error << L"Unable to resolve all dependencies" << Endl;
				return ERROR_UNABLE_TO_RESOLVE_DEPENDENCIES;
			}
		}
	}

	const RefArray< Aggregation >& aggregations = solution->getAggregations();
	for (RefArray< Aggregation >::const_iterator i = aggregations.begin(); i != aggregations.end(); ++i)
	{
		const RefArray< Dependency >& dependencies = (*i)->getDependencies();
		for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
		{
			if (!(*j)->resolve(&solutionLoader))
			{
				traktor::log::error << L"Unable to resolve all dependencies" << Endl;
				return ERROR_UNABLE_TO_RESOLVE_DEPENDENCIES;
			}
		}
	}

	if (!builder || !builder->create(cmdLine))
	{
		traktor::log::error << L"Unable to create solution builder" << Endl;
		return ERROR_UNABLE_TO_CREATE_BUILDER;
	}

	traktor::log::info << L"Generating target solution..." << Endl;

	if (!builder->generate(solution))
	{
		traktor::log::error << L"Unable to generate target solution" << Endl;
		return ERROR_UNABLE_TO_CREATE_SOLUTION;
	}

	timer.stop();
	
	traktor::log::info << L"Target solution created successfully, " << timer.getElapsedTime() << L" second(s)" << Endl;
	return 0;
}

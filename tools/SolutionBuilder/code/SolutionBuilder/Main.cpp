#include <Core/Io/Path.h>
#include <Core/Timer/Timer.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/TString.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Make/SolutionBuilderMake.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilderLIB/Msvc2005/SolutionBuilderMsvc2005.h"
#include "SolutionBuilderLIB/Xcode/SolutionBuilderXcode.h"
#include "SolutionBuilderLIB/SolutionLoader.h"

using namespace traktor;

#define TITLE L"SolutionBuilder v1.7"

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

	if (cmdLine.hasOption('f'))
	{
		std::wstring ide = cmdLine.getOption('f').getString();
		if (ide == L"make")
			builder = gc_new< SolutionBuilderMake >();
		else if (ide == L"msvc")
			builder = gc_new< SolutionBuilderMsvc >();
		else if (ide == L"msvc2k5")
			builder = gc_new< SolutionBuilderMsvc2005 >();
		else if (ide == L"xcode")
			builder = gc_new< SolutionBuilderXcode >();
		else
		{
			traktor::log::error << L"Unknown format \"" << ide << L"\"" << Endl;
			return ERROR_UNKNOWN_FORMAT;
		}
	}
	else
		builder = gc_new< SolutionBuilderMsvc >();

	if (cmdLine.hasOption('?') || cmdLine.hasOption('h') || cmdLine.getCount() <= 0)
	{
		traktor::log::info << L"Usage : " << Path(cmdLine.getFile()).getFileName() << L" -[options] [solution]" << Endl;
		traktor::log::info << L"\t-f=[format]	[\"msvc\"*, \"msvc2k5\", \"make\", \"xcode\"]" << Endl;
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

	traktor::log::info << L"Resolving dependencies..." << Endl;

	const RefList< Project >& projects = solution->getProjects();
	for (RefList< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefList< Dependency >& dependencies = (*i)->getDependencies();
		for (RefList< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
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

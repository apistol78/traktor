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
#include "SolutionBuilderLIB/Dependencies/SolutionBuilderDependencies.h"
#include "SolutionBuilderLIB/Eclipse/SolutionBuilderEclipse.h"
#include "SolutionBuilderLIB/GraphViz/SolutionBuilderGraphViz.h"
#include "SolutionBuilderLIB/Make/SolutionBuilderMake.h"
#include "SolutionBuilderLIB/Make/SolutionBuilderMake2.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilderLIB/Xcode/SolutionBuilderXcode.h"

using namespace traktor;

#define TITLE L"SolutionBuilder v3.1.3"

#define ERROR_UNKNOWN_FORMAT 1
#define ERROR_UNABLE_TO_READ_SOLUTION 2
#define ERROR_UNABLE_TO_RESOLVE_DEPENDENCIES 3
#define ERROR_UNABLE_TO_CREATE_BUILDER 4
#define ERROR_UNABLE_TO_CREATE_SOLUTION 5

void flattenIncludePaths(Project* project, std::map< std::wstring, std::set< std::wstring > >& outConfigurationIncludePaths)
{
	const RefArray< Configuration >& configurations = project->getConfigurations();
	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		const std::vector< std::wstring >& includePaths = (*i)->getIncludePaths();
		outConfigurationIncludePaths[(*i)->getName()].insert(
			includePaths.begin(),
			includePaths.end()
		);
	}

	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (!(*i)->getInheritIncludePaths())
			continue;

		if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*i))
			flattenIncludePaths(projectDependency->getProject(), outConfigurationIncludePaths);
		else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*i))
			flattenIncludePaths(externalDependency->getProject(), outConfigurationIncludePaths);
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
		else if (ide == L"eclipse")
			builder = new SolutionBuilderEclipse();
		else if (ide == L"graphviz")
			builder = new SolutionBuilderGraphViz();
		else if (ide == L"make")
			builder = new SolutionBuilderMake();
		else if (ide == L"make2")
			builder = new SolutionBuilderMake2();
		else if (ide == L"msvc")
			builder = new SolutionBuilderMsvc();
		else if (ide == L"xcode")
			builder = new SolutionBuilderXcode();
		else if (ide == L"dependencies")
			builder = new SolutionBuilderDependencies();
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
		traktor::log::info << TITLE << Endl;
		traktor::log::info << L"Usage : " << Path(cmdLine.getFile()).getFileName() << L" -[options] [solution]" << Endl;
		traktor::log::info << L"\t-f,-format=[format]	[\"cblocks\", \"eclipse\", \"graphviz\", \"msvc\"*, \"make\", \"make2\", \"xcode\", \"dependencies\"]" << Endl;
		traktor::log::info << L"\t-rootPath=Path		Override solution root path" << Endl;
		traktor::log::info << L"\t-v,verbose			Verbose" << Endl;
		if (builder)
			builder->showOptions();
		return 0;
	}

	Timer timer;
	timer.start();

	SolutionLoader solutionLoader;

	if (cmdLine.hasOption('v', L"verbose"))
	{
		traktor::log::info << TITLE << Endl;
		traktor::log::info << L"Loading solution \"" << cmdLine.getString(0) << L"\"..." << Endl;
	}

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

	if (cmdLine.hasOption('v', L"verbose"))
	{
		traktor::log::info << L"Using root path \"" << solution->getRootPath() << L"\"" << Endl;
		traktor::log::info << L"Resolving dependencies..." << Endl;
	}

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

	if (cmdLine.hasOption('v', L"verbose"))
		traktor::log::info << L"Flatten include paths..." << Endl;

	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		std::map< std::wstring, std::set< std::wstring > > configurationIncludePaths;
		flattenIncludePaths(*i, configurationIncludePaths);

		const RefArray< Configuration >& configurations = (*i)->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			const std::set< std::wstring >& includePaths = configurationIncludePaths[(*j)->getName()];

			if (cmdLine.hasOption('v', L"verbose"))
			{
				traktor::log::info << L"Include paths of \"" << (*i)->getName() << L"\" " << (*j)->getName() << Endl;
				for (std::set< std::wstring >::const_iterator k = includePaths.begin(); k != includePaths.end(); ++k)
					traktor::log::info << L"\t" << *k << Endl;
			}

			(*j)->setIncludePaths(std::vector< std::wstring >(
				includePaths.begin(),
				includePaths.end()
			));
		}
	}

	if (!builder || !builder->create(cmdLine))
	{
		traktor::log::error << L"Unable to create solution builder" << Endl;
		return ERROR_UNABLE_TO_CREATE_BUILDER;
	}

	if (cmdLine.hasOption('v', L"verbose"))
		traktor::log::info << L"Generating target solution..." << Endl;

	if (!builder->generate(solution))
	{
		traktor::log::error << L"Unable to generate target solution" << Endl;
		return ERROR_UNABLE_TO_CREATE_SOLUTION;
	}

	timer.stop();
	
	if (cmdLine.hasOption('v', L"verbose"))
		traktor::log::info << L"Target solution created successfully, " << timer.getElapsedTime() << L" second(s)" << Endl;

	return 0;
}

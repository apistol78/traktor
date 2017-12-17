/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Dependencies/SolutionBuilderDependencies.h"

namespace traktor
{
	namespace sb
	{
		namespace
		{

void collectDependencies(const Project* project, std::set< std::wstring >& outDependencies)
{
	outDependencies.insert(project->getName());

	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
	{
		if (const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j))
			collectDependencies(projectDependency->getProject(), outDependencies);
		else if (const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(*j))
			collectDependencies(externalDependency->getProject(), outDependencies);
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderDependencies", SolutionBuilderDependencies, SolutionBuilder)

bool SolutionBuilderDependencies::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption(L'p', L"project"))
		m_projectName = cmdLine.getOption(L'p', L"project").getString();

	return true;
}

bool SolutionBuilderDependencies::generate(Solution* solution)
{
	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const Project* project = *i;
		T_FATAL_ASSERT (project);

		if (m_projectName.empty() || project->getName() == m_projectName)
		{
			std::set< std::wstring > dependencies;
			collectDependencies(project, dependencies);
			for (std::set< std::wstring >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
				log::info << *i << Endl;
			return true;
		}
	}
	return false;
}

void SolutionBuilderDependencies::showOptions() const
{
	log::info << L"\t-p,-project=Dependencies of project" << Endl;
}

	}
}

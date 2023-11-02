/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <set>
#include "Core/Log/Log.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Dependencies/SolutionBuilderDependencies.h"

namespace traktor::sb
{
	namespace
	{

void collectDependencies(const Project* project, std::set< std::wstring >& outDependencies)
{
	outDependencies.insert(project->getName());
	for (auto dependency : project->getDependencies())
	{
		if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
			collectDependencies(projectDependency->getProject(), outDependencies);
		else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
			collectDependencies(externalDependency->getProject(), outDependencies);
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionBuilderDependencies", SolutionBuilderDependencies, SolutionBuilder)

bool SolutionBuilderDependencies::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption(L'p', L"project"))
		m_projectName = cmdLine.getOption(L'p', L"project").getString();

	return true;
}

bool SolutionBuilderDependencies::generate(const Solution* solution)
{
	for (auto project : solution->getProjects())
	{
		if (m_projectName.empty() || project->getName() == m_projectName)
		{
			std::set< std::wstring > dependencies;
			collectDependencies(project, dependencies);
			for (const auto& dependency : dependencies)
				log::info << dependency << Endl;
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

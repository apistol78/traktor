/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Misc/String.h>
#include <Core/Serialization/DeepClone.h>
#include <Xml/XmlSerializer.h>
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Editor/App/ExtractSolutionTool.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ExtractSolutionTool", ExtractSolutionTool, CustomTool)

bool ExtractSolutionTool::execute(ui::Widget* parent, Solution* solution)
{
	const std::wstring externalSolutionFileName = L"$(TRAKTOR_HOME)/ExternAndroid.xms";

	Ref< IStream > file = FileSystem::getInstance().open(externalSolutionFileName, traktor::File::FmWrite);
	if (!file)
		return false;

	// Split projects into two arrays, local and external.
	RefArray< Project > localProjects = solution->getProjects();
	RefArray< Project > externalProjects;

	for (size_t i = 0; i < localProjects.size(); )
	{
		auto project = localProjects[i];
		if (startsWith< std::wstring >(project->getName(), L"Extern."))
		{
			externalProjects.push_back(project);
			localProjects.erase(localProjects.begin() + i);
		}
		else
			++i;
	}

	if (externalProjects.empty())
		return false;

	// Create external solution.
	Ref< Solution > externalSolution = new Solution();
	externalSolution->setName(L"Extern Win64");
	externalSolution->setRootPath(solution->getRootPath());
	externalSolution->setDefinitions(solution->getDefinitions());

	// Replace dependencies to projects which will become external projects.
	for (auto localProject : localProjects)
	{
		RefArray< Dependency > dependencies = localProject->getDependencies();
		for (RefArray< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
		{
			auto ld = dynamic_type_cast< ProjectDependency* >(*i);
			if (!ld)
				continue;

			for (auto externalProject : externalProjects)
			{
				if (ld->getProject() == externalProject)
				{
					Ref< ExternalDependency > externalDependency = new ExternalDependency(
						externalSolutionFileName,
						externalProject->getName()
					);

					externalDependency->setInheritIncludePaths(ld->getInheritIncludePaths());
					externalDependency->setLink(ld->getLink());

					*i = externalDependency;
					break;
				}
			}
		}
		localProject->setDependencies(dependencies);
	}

	// Replace solution projects with locals only.
	solution->setProjects(localProjects);

	// Add external projects to external solution.
	externalSolution->setProjects(externalProjects);

	// Save external solution.
	xml::XmlSerializer(file).writeObject(externalSolution);
	file->close();

	return true;
}

	}
}

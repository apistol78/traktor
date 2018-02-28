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
#include "SolutionBuilder/Editor/App/ExtractSolutionDialog.h"
#include "SolutionBuilder/Editor/App/ExtractSolutionTool.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ExtractSolutionTool", ExtractSolutionTool, CustomTool)

bool ExtractSolutionTool::execute(ui::Widget* parent, Solution* solution)
{
	Ref< ExtractSolutionDialog > dialog = new ExtractSolutionDialog();
	
	if (!dialog->create(parent, solution))
		return false;

	if (dialog->showModal() != ui::DrOk)
		return false;

	const std::wstring externalSolutionFileName = dialog->getSolutionFile();
	if (externalSolutionFileName.empty())
		return false;

	const std::wstring externalSolutionName = dialog->getSolutionName();
	if (externalSolutionName.empty())
		return false;

	RefArray< Project > externalProjects;
	dialog->getSelectedProjects(externalProjects);
	if (externalProjects.empty())
		return false;

	// Get projects to keep in current solution.
	RefArray< Project > localProjects;
	for (auto project : solution->getProjects())
	{
		if (std::find(externalProjects.begin(), externalProjects.end(), project) != externalProjects.end())
			continue;

		localProjects.push_back(project);
	}
	if (localProjects.empty())
		return false;

	Ref< IStream > file = FileSystem::getInstance().open(externalSolutionFileName, traktor::File::FmWrite);
	if (!file)
		return false;

	// Create external solution.
	Ref< Solution > externalSolution = new Solution();
	externalSolution->setName(externalSolutionName);
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

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Editor/App/FlattenDefinitionsTool.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.FlattenDefinitionsTool", FlattenDefinitionsTool, CustomTool)

bool FlattenDefinitionsTool::execute(ui::Widget* parent, Solution* solution)
{
	std::vector< std::wstring > globalDefinitions = solution->getDefinitions();

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Configuration >& configurations = (*i)->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			std::vector< std::wstring > definitions = (*j)->getDefinitions();
			definitions.insert(definitions.begin(), globalDefinitions.begin(), globalDefinitions.end());
			(*j)->setDefinitions(definitions);
		}
	}

	solution->setDefinitions(std::vector< std::wstring >());
	return true;
}

	}
}

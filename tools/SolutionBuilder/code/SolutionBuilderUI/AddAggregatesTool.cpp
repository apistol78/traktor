/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Misc/String.h>
#include "SolutionBuilderLIB/AggregationItem.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderUI/AddAggregatesTool.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"AddAggregatesTool", AddAggregatesTool, CustomTool)

bool AddAggregatesTool::execute(ui::Widget* parent, Solution* solution)
{
	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Configuration >& configurations = (*i)->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			if ((*j)->getAggregationItems().empty())
			{
				std::wstring suffix = L"";

				switch ((*j)->getTargetFormat())
				{
				case Configuration::TfStaticLibrary:
					if (
						compareIgnoreCase< std::wstring >((*j)->getName(), L"DebugStatic") == 0 ||
						compareIgnoreCase< std::wstring >((*j)->getName(), L"ReleaseStatic") == 0
					)
						suffix = L".lib";
					break;
				case Configuration::TfSharedLibrary:
					suffix = L".dll";
					break;
				case Configuration::TfExecutable:
					suffix = L".exe";
					break;
				case Configuration::TfExecutableConsole:
					suffix = L".exe";
					break;
				}

				if (!suffix.empty())
				{
					Ref< AggregationItem > a = new AggregationItem();
					a->setSourceFile((*i)->getName() + suffix);
					a->setTargetPath(L"$(AGGREGATE_OUTPUT_PATH)/" + toLower((*j)->getName()));
					(*j)->addAggregationItem(a);
				}
			}
		}
	}
	return true;
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Misc/String.h>
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Editor/App/AddAggregatesTool.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.AddAggregatesTool", AddAggregatesTool, CustomTool)

bool AddAggregatesTool::execute(ui::Widget* parent, Solution* solution)
{
	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Configuration >& configurations = (*i)->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			if (
				(*j)->getTargetFormat() == Configuration::TfStaticLibrary ||
				(*j)->getTargetFormat() == Configuration::TfSharedLibrary
			)
			{
				// \hack Add aggregation output path as consumer library path.
				if ((*j)->getConsumerLibraryPath().empty())
					(*j)->setConsumerLibraryPath(toLower((*j)->getName()));
			}

			if (!(*j)->getAggregationItems().empty())
			{
				// \hack Add .lib rule first if missing in existing .dll aggregates.
				std::wstring sourceFile = (*j)->getAggregationItems().front()->getSourceFile();
				if (compareIgnoreCase< std::wstring >(sourceFile, (*i)->getName() + L".dll") == 0)
				{
					RefArray< AggregationItem > items = (*j)->getAggregationItems();

					Ref< AggregationItem > a = new AggregationItem();
					a->setSourceFile((*i)->getName() + L".lib");
					a->setTargetPath(toLower((*j)->getName()));
					items.push_front(a);

					(*j)->setAggregationItems(items);
				}

				if ((*j)->getTargetFormat() == Configuration::TfStaticLibrary)
				{
					if (!(
						compareIgnoreCase< std::wstring >((*j)->getName(), L"DebugStatic") == 0 ||
						compareIgnoreCase< std::wstring >((*j)->getName(), L"ReleaseStatic") == 0
					))
					{
						(*j)->setConsumerLibraryPath(L"");
					}
				}
			}
			else
			{
				switch ((*j)->getTargetFormat())
				{
				case Configuration::TfStaticLibrary:
					if (
						compareIgnoreCase< std::wstring >((*j)->getName(), L"DebugStatic") == 0 ||
						compareIgnoreCase< std::wstring >((*j)->getName(), L"ReleaseStatic") == 0
					)
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile((*i)->getName() + L".lib");
						a->setTargetPath(toLower((*j)->getName()));
						(*j)->addAggregationItem(a);
					}
					else
						(*j)->setConsumerLibraryPath(L"");
					break;
				case Configuration::TfSharedLibrary:
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile((*i)->getName() + L".lib");
						a->setTargetPath(toLower((*j)->getName()));
						(*j)->addAggregationItem(a);
					}
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile((*i)->getName() + L".dll");
						a->setTargetPath(toLower((*j)->getName()));
						(*j)->addAggregationItem(a);
					}
					break;
				case Configuration::TfExecutable:
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile((*i)->getName() + L".exe");
						a->setTargetPath(toLower((*j)->getName()));
						(*j)->addAggregationItem(a);
					}
					break;
				case Configuration::TfExecutableConsole:
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile((*i)->getName() + L".exe");
						a->setTargetPath(toLower((*j)->getName()));
						(*j)->addAggregationItem(a);
					}
					break;
				}
			}
		}
	}
	return true;
}

	}
}

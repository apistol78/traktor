#include "Core/Misc/String.h"
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
	for (auto project : solution->getProjects())
	{
		for (auto configuration : project->getConfigurations())
		{
			if (
				configuration->getTargetFormat() == Configuration::TfStaticLibrary ||
				configuration->getTargetFormat() == Configuration::TfSharedLibrary
			)
			{
				// \hack Add aggregation output path as consumer library path.
				if (configuration->getConsumerLibraryPath().empty())
					configuration->setConsumerLibraryPath(toLower(configuration->getName()));
			}

			if (!configuration->getAggregationItems().empty())
			{
				// \hack Add .lib rule first if missing in existing .dll aggregates.
				std::wstring sourceFile = configuration->getAggregationItems().front()->getSourceFile();
				if (compareIgnoreCase(sourceFile, project->getName() + L".dll") == 0)
				{
					RefArray< AggregationItem > items = configuration->getAggregationItems();

					Ref< AggregationItem > a = new AggregationItem();
					a->setSourceFile(project->getName() + L".lib");
					a->setTargetPath(toLower(configuration->getName()));
					items.push_front(a);

					configuration->setAggregationItems(items);
				}
				else if (compareIgnoreCase(sourceFile, project->getName() + L".lib") == 0)
				{
					RefArray< AggregationItem > items = configuration->getAggregationItems();

					Ref< AggregationItem > a = new AggregationItem();
					a->setSourceFile(project->getName() + L".pdb");
					a->setTargetPath(toLower(configuration->getName()));
					items.push_front(a);

					configuration->setAggregationItems(items);
				}

				if (configuration->getTargetFormat() == Configuration::TfStaticLibrary)
				{
					if (!(
						compareIgnoreCase(configuration->getName(), L"DebugStatic") == 0 ||
						compareIgnoreCase(configuration->getName(), L"ReleaseStatic") == 0
					))
					{
						configuration->setConsumerLibraryPath(L"");
					}
				}
			}
			else
			{
				switch (configuration->getTargetFormat())
				{
				case Configuration::TfStaticLibrary:
					if (
						compareIgnoreCase(configuration->getName(), L"DebugStatic") == 0 ||
						compareIgnoreCase(configuration->getName(), L"ReleaseStatic") == 0
					)
					{
						{
							Ref< AggregationItem > a = new AggregationItem();
#if defined(__LINUX__) || defined(__RPI__)
							a->setSourceFile(L"lib" + project->getName() + L".a");
#else
							a->setSourceFile(project->getName() + L".lib");
#endif
							a->setTargetPath(toLower(configuration->getName()));
							configuration->addAggregationItem(a);
						}
#if !defined(__LINUX__) && !defined(__RPI__)
						{
							Ref< AggregationItem > a = new AggregationItem();
							a->setSourceFile(project->getName() + L".pdb");
							a->setTargetPath(toLower(configuration->getName()));
							configuration->addAggregationItem(a);
						}
#endif
					}
					else
						configuration->setConsumerLibraryPath(L"");
					break;

				case Configuration::TfSharedLibrary:
					{
						Ref< AggregationItem > a = new AggregationItem();
#if defined(__LINUX__) || defined(__RPI__)					
						a->setSourceFile(L"lib" + project->getName() + L".so");
#else
						a->setSourceFile(project->getName() + L".lib");
#endif
						a->setTargetPath(toLower(configuration->getName()));
						configuration->addAggregationItem(a);
					}
#if !defined(__LINUX__) && !defined(__RPI__)
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile(project->getName() + L".dll");
						a->setTargetPath(toLower(configuration->getName()));
						configuration->addAggregationItem(a);
					}
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile(project->getName() + L".pdb");
						a->setTargetPath(toLower(configuration->getName()));
						configuration->addAggregationItem(a);
					}
#endif
					break;

				case Configuration::TfExecutable:
				case Configuration::TfExecutableConsole:
					{
						Ref< AggregationItem > a = new AggregationItem();
#if defined(__LINUX__) || defined(__RPI__)					
						a->setSourceFile(project->getName());
#else
						a->setSourceFile(project->getName() + L".exe");
#endif
						a->setTargetPath(toLower(configuration->getName()));
						configuration->addAggregationItem(a);
					}
#if !defined(__LINUX__) && !defined(__RPI__)
					{
						Ref< AggregationItem > a = new AggregationItem();
						a->setSourceFile(project->getName() + L".pdb");
						a->setTargetPath(toLower(configuration->getName()));
						configuration->addAggregationItem(a);
					}
#endif
					break;
				}
			}
		}
	}
	return true;
}

	}
}

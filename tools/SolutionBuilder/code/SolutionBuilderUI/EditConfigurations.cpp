/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Config.h>
#include <Core/Misc/Split.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>
#include "EditConfigurations.h"
#include "ConfigurationsDialog.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Configuration.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"EditConfigurations", EditConfigurations, CustomTool)

namespace
{

	std::wstring buildExportDefinition(const std::wstring& projectName)
	{
		std::vector< std::wstring > projectNameParts;
		if (Split< std::wstring >::any(projectName, L".", projectNameParts) <= 1)
			return L"";

		std::wstringstream ss;
		ss << L"T_";

		std::vector< std::wstring >::iterator i = projectNameParts.begin() + 1;
		while (i != projectNameParts.end())
			ss << toUpper(*i++) << L"_";

		ss << L"EXPORT";

		return ss.str();
	}

}

bool EditConfigurations::execute(traktor::ui::Widget* parent, Solution* solution)
{
	ConfigurationsDialog configurationsDialog;
	configurationsDialog.create(parent, solution);

	if (configurationsDialog.showModal() == ui::DrOk)
	{
		const std::vector< ConfigurationsDialog::Action >& actions = configurationsDialog.getActions();
		for (std::vector< ConfigurationsDialog::Action >::const_iterator i = actions.begin(); i != actions.end(); ++i)
		{
			const RefArray< Project >& projects = solution->getProjects();
			for (RefArray< Project >::const_iterator j = projects.begin(); j != projects.end(); ++j)
			{
				if (i->type == ConfigurationsDialog::AtNew)
				{
					if ((*j)->getConfiguration(i->name))
						continue;

					Ref< Configuration > templateConfiguration = (*j)->getConfiguration(i->current);
					if (!templateConfiguration)
						traktor::log::warning << L"Unable to find template configuration \"" << i->current << L"\" for project \"" << (*j)->getName() << L"\"" << Endl;

					Ref< Configuration > configuration = new Configuration();
					configuration->setName(i->name);
					if (templateConfiguration)
					{
						// Clone settings from template configuration.
						configuration->setTargetFormat(templateConfiguration->getTargetFormat());
						configuration->setTargetProfile(templateConfiguration->getTargetProfile());
						configuration->setPrecompiledHeader(templateConfiguration->getPrecompiledHeader());
						configuration->setIncludePaths(templateConfiguration->getIncludePaths());
						configuration->setDefinitions(templateConfiguration->getDefinitions());
						configuration->setLibraryPaths(templateConfiguration->getLibraryPaths());
						configuration->setLibraries(templateConfiguration->getLibraries());
					}
					else
					{
						// No template configuration, set default values.
						configuration->setTargetFormat(Configuration::TfSharedLibrary);
						configuration->setTargetProfile(Configuration::TpRelease);
						configuration->addIncludePath(L"$(TRAKTOR_HOME)/code");
						configuration->addDefinition(buildExportDefinition((*j)->getName()));
						configuration->addDefinition(L"NDEBUG");
					}
					(*j)->addConfiguration(configuration);						
				}
				else if (i->type == ConfigurationsDialog::AtRename)
				{
					Ref< Configuration > configuration = (*j)->getConfiguration(i->current);
					if (!configuration)
						continue;

					configuration->setName(i->name);
				}
				else if (i->type == ConfigurationsDialog::AtRemove)
				{
					Ref< Configuration > configuration = (*j)->getConfiguration(i->name);
					if (configuration)
						(*j)->removeConfiguration(configuration);
				}
			}
		}
	}

	configurationsDialog.destroy();
	return true;
}

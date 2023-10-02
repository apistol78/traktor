/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Config.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"
#include "EditConfigurations.h"
#include "ConfigurationsDialog.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Configuration.h"

namespace traktor
{
	namespace sb
	{
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.EditConfigurations", EditConfigurations, CustomTool)

bool EditConfigurations::execute(ui::Widget* parent, Solution* solution)
{
	ConfigurationsDialog configurationsDialog;
	configurationsDialog.create(parent, solution);

	if (configurationsDialog.showModal() == ui::DialogResult::Ok)
	{
		for (const auto& action : configurationsDialog.getActions())
		{
			for (auto project : solution->getProjects())
			{
				if (action.type == ConfigurationsDialog::AtNew)
				{
					if (project->getConfiguration(action.name))
						continue;

					Ref< Configuration > templateConfiguration = project->getConfiguration(action.current);
					if (!templateConfiguration)
						log::warning << L"Unable to find template configuration \"" << action.current << L"\" for project \"" << project->getName() << L"\"" << Endl;

					Ref< Configuration > configuration = new Configuration();
					configuration->setName(action.name);
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
						configuration->addDefinition(buildExportDefinition(project->getName()));
					}
					project->addConfiguration(configuration);
				}
				else if (action.type == ConfigurationsDialog::AtRename)
				{
					Ref< Configuration > configuration = project->getConfiguration(action.current);
					if (!configuration)
						continue;

					configuration->setName(action.name);
				}
				else if (action.type == ConfigurationsDialog::AtRemove)
				{
					Ref< Configuration > configuration = project->getConfiguration(action.name);
					if (configuration)
						project->removeConfiguration(configuration);
				}
			}
		}
	}

	configurationsDialog.destroy();
	return true;
}

	}
}

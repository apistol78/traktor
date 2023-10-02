/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sstream>
#include "Core/Config.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Editor/App/AddMultipleConfigurations.h"

namespace traktor
{
	namespace sb
	{
		namespace
		{

bool hasConfiguration(const Project* project, const std::wstring& configurationName)
{
	for (auto configuration : project->getConfigurations())
	{
		if (configuration->getName() == configurationName)
			return true;
	}
	return false;
}

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.AddMultipleConfigurations", AddMultipleConfigurations, CustomTool)

bool AddMultipleConfigurations::execute(ui::Widget* parent, Solution* solution)
{
	for (auto project : solution->getProjects())
	{
		if (!hasConfiguration(project, L"DebugShared"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"DebugShared");
			configuration->setTargetFormat(Configuration::TfSharedLibrary);
			configuration->setTargetProfile(Configuration::TpDebug);
			configuration->addDefinition(buildExportDefinition(project->getName()));
			project->addConfiguration(configuration);
		}
		if (!hasConfiguration(project, L"ReleaseShared"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"ReleaseShared");
			configuration->setTargetFormat(Configuration::TfSharedLibrary);
			configuration->setTargetProfile(Configuration::TpRelease);
			configuration->addDefinition(buildExportDefinition(project->getName()));
			project->addConfiguration(configuration);
		}
		if (!hasConfiguration(project, L"DebugStatic"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"DebugStatic");
			configuration->setTargetFormat(Configuration::TfStaticLibrary);
			configuration->setTargetProfile(Configuration::TpDebug);
			project->addConfiguration(configuration);
		}
		if (!hasConfiguration(project, L"ReleaseStatic"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"ReleaseStatic");
			configuration->setTargetFormat(Configuration::TfStaticLibrary);
			configuration->setTargetProfile(Configuration::TpRelease);
			project->addConfiguration(configuration);
		}
	}
	return true;
}

	}
}

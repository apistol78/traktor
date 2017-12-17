/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sstream>
#include <Core/Config.h>
#include <Core/Misc/Split.h>
#include <Core/Misc/String.h>
#include "AddMultipleConfigurations.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Configuration.h"

namespace traktor
{
	namespace sb
	{
		namespace
		{

bool hasConfiguration(const Project* project, const std::wstring& configurationName)
{
	const RefArray< Configuration >& configurations = project->getConfigurations();
	for (RefArray< Configuration >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		if ((*i)->getName() == configurationName)
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
	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if (!hasConfiguration(*i, L"DebugShared"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"DebugShared");
			configuration->setTargetFormat(Configuration::TfSharedLibrary);
			configuration->setTargetProfile(Configuration::TpDebug);
			configuration->addIncludePath(L"$(TRAKTOR_HOME)/code");
			configuration->addDefinition(buildExportDefinition((*i)->getName()));
			configuration->addDefinition(L"_DEBUG");
			(*i)->addConfiguration(configuration);
		}
		if (!hasConfiguration(*i, L"ReleaseShared"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"ReleaseShared");
			configuration->setTargetFormat(Configuration::TfSharedLibrary);
			configuration->setTargetProfile(Configuration::TpRelease);
			configuration->addIncludePath(L"$(TRAKTOR_HOME)/code");
			configuration->addDefinition(buildExportDefinition((*i)->getName()));
			configuration->addDefinition(L"NDEBUG");
			(*i)->addConfiguration(configuration);
		}
		if (!hasConfiguration(*i, L"DebugStatic"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"DebugStatic");
			configuration->setTargetFormat(Configuration::TfStaticLibrary);
			configuration->setTargetProfile(Configuration::TpDebug);
			configuration->addIncludePath(L"$(TRAKTOR_HOME)/code");
			configuration->addDefinition(L"T_STATIC");
			configuration->addDefinition(L"_DEBUG");
			(*i)->addConfiguration(configuration);
		}
		if (!hasConfiguration(*i, L"ReleaseStatic"))
		{
			Ref< Configuration > configuration = new Configuration();
			configuration->setName(L"ReleaseStatic");
			configuration->setTargetFormat(Configuration::TfStaticLibrary);
			configuration->setTargetProfile(Configuration::TpRelease);
			configuration->addIncludePath(L"$(TRAKTOR_HOME)/code");
			configuration->addDefinition(L"T_STATIC");
			configuration->addDefinition(L"NDEBUG");
			(*i)->addConfiguration(configuration);
		}
	}
	return true;
}

	}
}

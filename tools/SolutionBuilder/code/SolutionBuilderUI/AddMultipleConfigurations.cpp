#include <sstream>
#include <Core/Config.h>
#include <Core/Misc/SplitString.h>
#include <Core/Misc/StringUtils.h>
#include "AddMultipleConfigurations.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Configuration.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"AddMultipleConfigurations", AddMultipleConfigurations, CustomTool)

namespace
{

	bool hasConfiguration(Project* project, const std::wstring& configurationName)
	{
		RefList< Configuration >& configurations = project->getConfigurations();
		for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
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

bool AddMultipleConfigurations::execute(ui::Widget* parent, Solution* solution)
{
	RefList< Project >& projects = solution->getProjects();
	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if (!hasConfiguration(*i, L"DebugShared"))
		{
			Ref< Configuration > configuration = gc_new< Configuration >();
			configuration->setName(L"DebugShared");
			configuration->setTargetFormat(Configuration::TfSharedLibrary);
			configuration->setTargetProfile(Configuration::TpDebug);
			configuration->getIncludePaths().push_back(L"$(TRAKTOR_HOME)/code");
			configuration->getDefinitions().push_back(buildExportDefinition((*i)->getName()));
			configuration->getDefinitions().push_back(L"_DEBUG");
			(*i)->addConfiguration(configuration);
		}
		if (!hasConfiguration(*i, L"ReleaseShared"))
		{
			Ref< Configuration > configuration = gc_new< Configuration >();
			configuration->setName(L"ReleaseShared");
			configuration->setTargetFormat(Configuration::TfSharedLibrary);
			configuration->setTargetProfile(Configuration::TpRelease);
			configuration->getIncludePaths().push_back(L"$(TRAKTOR_HOME)/code");
			configuration->getDefinitions().push_back(buildExportDefinition((*i)->getName()));
			configuration->getDefinitions().push_back(L"NDEBUG");
			(*i)->addConfiguration(configuration);
		}
		if (!hasConfiguration(*i, L"DebugStatic"))
		{
			Ref< Configuration > configuration = gc_new< Configuration >();
			configuration->setName(L"DebugStatic");
			configuration->setTargetFormat(Configuration::TfStaticLibrary);
			configuration->setTargetProfile(Configuration::TpDebug);
			configuration->getIncludePaths().push_back(L"$(TRAKTOR_HOME)/code");
			configuration->getDefinitions().push_back(L"T_STATIC");
			configuration->getDefinitions().push_back(L"_DEBUG");
			(*i)->addConfiguration(configuration);
		}
		if (!hasConfiguration(*i, L"ReleaseStatic"))
		{
			Ref< Configuration > configuration = gc_new< Configuration >();
			configuration->setName(L"ReleaseStatic");
			configuration->setTargetFormat(Configuration::TfStaticLibrary);
			configuration->setTargetProfile(Configuration::TpRelease);
			configuration->getIncludePaths().push_back(L"$(TRAKTOR_HOME)/code");
			configuration->getDefinitions().push_back(L"T_STATIC");
			configuration->getDefinitions().push_back(L"NDEBUG");
			(*i)->addConfiguration(configuration);
		}
	}
	return true;
}

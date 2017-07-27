/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberComposite.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXDefinition.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Configuration.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcVCXDefinition", 0, SolutionBuilderMsvcVCXDefinition, ISerializable)

bool SolutionBuilderMsvcVCXDefinition::generate(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	const Configuration* configuration,
	OutputStream& os
) const
{
	StringOutputStream ssip, ssd, ssl, sslp;

	const std::vector< std::wstring >& includePaths = configuration->getIncludePaths();
	for (std::vector< std::wstring >::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
		ssip << *i << L";";

	for (std::vector< std::wstring >::const_iterator i = solution->getDefinitions().begin(); i != solution->getDefinitions().end(); ++i)
		ssd << *i << L";";
	for (std::vector< std::wstring >::const_iterator i = configuration->getDefinitions().begin(); i != configuration->getDefinitions().end(); ++i)
		ssd << *i << L";";

	std::set< std::wstring > libraries, libraryPaths;
	collectAdditionalLibraries(project, configuration, libraries, libraryPaths);

	for (std::set< std::wstring >::const_iterator i = libraries.begin(); i != libraries.end(); ++i)
		ssl << *i << L";";

	for (std::set< std::wstring >::const_iterator i = libraryPaths.begin(); i != libraryPaths.end(); ++i)
		sslp << *i << L";";

	context.set(L"PROJECT_NAME", project->getName());
	context.set(L"PROJECT_INCLUDE_PATHS", ssip.str());
	context.set(L"PROJECT_DEFINITIONS", ssd.str());
	context.set(L"PROJECT_LIBRARIES", ssl.str());
	context.set(L"PROJECT_LIBRARY_PATHS", sslp.str());

	const wchar_t* c_warningLevels[] =
	{
		L"TurnOffAllWarnings",	// WlNoWarnings
		L"Level1",				// WlCriticalOnly
		L"Level3",				// WlCompilerDefault
		L"EnableAllWarnings",	// WlAllWarnings
	};

	context.set(L"PROJECT_WARNING_LEVEL", c_warningLevels[configuration->getWarningLevel()]);

	std::wstring aco = configuration->getAdditionalCompilerOptions();
	if (!aco.empty())
		context.set(L"PROJECT_ADDITIONAL_COMPILER_OPTIONS", aco + L" ");
	else
		context.set(L"PROJECT_ADDITIONAL_COMPILER_OPTIONS", L"");

	std::wstring alo = configuration->getAdditionalLinkerOptions();
	if (!alo.empty())
		context.set(L"PROJECT_ADDITIONAL_LINKER_OPTIONS", alo + L" ");
	else
		context.set(L"PROJECT_ADDITIONAL_LINKER_OPTIONS", L"");

	context.set(L"MODULE_DEFINITION_FILE", L"");
	findDefinitions(context, solution, project, project->getItems());

	os << L"<" << m_name << L">" << Endl;
	os << IncreaseIndent;

	for (std::vector< Option >::const_iterator i = m_options.begin(); i != m_options.end(); ++i)
		os << L"<" << i->name << L">" << context.format(i->value) << L"</" << i->name << L">" << Endl;

	os << DecreaseIndent;
	os << L"</" << m_name << L">" << Endl;

	return true;
}

void SolutionBuilderMsvcVCXDefinition::serialize(traktor::ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"fileTypes", m_fileTypes);
	s >> MemberStlVector< Option, MemberComposite< Option > >(L"options", m_options);
}

void SolutionBuilderMsvcVCXDefinition::Option::serialize(traktor::ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< std::wstring >(L"value", value);
}

void SolutionBuilderMsvcVCXDefinition::collectAdditionalLibraries(
	const Project* project,
	const Configuration* configuration,
	std::set< std::wstring >& outAdditionalLibraries,
	std::set< std::wstring >& outAdditionalLibraryPaths
) const
{
	outAdditionalLibraries.insert(
		configuration->getLibraries().begin(),
		configuration->getLibraries().end()
	);

	outAdditionalLibraryPaths.insert(
		configuration->getLibraryPaths().begin(),
		configuration->getLibraryPaths().end()
	);

	const RefArray< Dependency >& dependencies = project->getDependencies();
	for (RefArray< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		// Traverse all static library dependencies and at their "additional libraries" as well.
		if (ProjectDependency* projectDependency = dynamic_type_cast< ProjectDependency* >(*i))
		{
			Configuration* dependentConfiguration = projectDependency->getProject()->getConfiguration(configuration->getName());
			if (!dependentConfiguration)
			{
				traktor::log::warning << L"Unable to add dependency \"" << projectDependency->getProject()->getName() << L"\", no matching configuration found" << Endl;
				continue;
			}

			if (dependentConfiguration->getTargetFormat() == Configuration::TfStaticLibrary)
			{
				collectAdditionalLibraries(
					projectDependency->getProject(),
					dependentConfiguration,
					outAdditionalLibraries,
					outAdditionalLibraryPaths
				);
			}
		}

		// Add products from external dependencies and their "additional libraries" as well.
		if (ExternalDependency* externalDependency = dynamic_type_cast< ExternalDependency* >(*i))
		{
			Ref< Configuration > externalConfiguration = externalDependency->getProject()->getConfiguration(configuration->getName());
			if (!externalConfiguration)
			{
				traktor::log::warning << L"Unable to add external dependency \"" << externalDependency->getProject()->getName() << L"\", no matching configuration found" << Endl;
				continue;
			}

			std::wstring externalRootPath = externalDependency->getSolution()->getRootPath();
			std::wstring externalProjectPath = externalRootPath + L"/" + toLower(externalConfiguration->getName());
			std::wstring externalProjectName = externalDependency->getProject()->getName() + L".lib";

			outAdditionalLibraries.insert(externalProjectName);
			outAdditionalLibraryPaths.insert(externalProjectPath);

			if (externalConfiguration->getTargetFormat() == Configuration::TfStaticLibrary)
			{
				collectAdditionalLibraries(
					externalDependency->getProject(),
					externalConfiguration,
					outAdditionalLibraries,
					outAdditionalLibraryPaths
				);
			}
		}
	}
}

void SolutionBuilderMsvcVCXDefinition::findDefinitions(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	const RefArray< ProjectItem >& items
) const
{
	Path rootPath = FileSystem::getInstance().getAbsolutePath(context.get(L"PROJECT_PATH"));

	for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (const ::File* file = dynamic_type_cast< const ::File* >(*i))
		{
			std::set< Path > systemFiles;
			file->getSystemFiles(project->getSourcePath(), systemFiles);
			for (std::set< Path >::iterator j = systemFiles.begin(); j != systemFiles.end(); ++j)
			{
				if (compareIgnoreCase< std::wstring >(j->getExtension(), L"def") == 0)
				{
					Path relativePath;
					FileSystem::getInstance().getRelativePath(
						*j,
						rootPath,
						relativePath
						);
					context.set(L"MODULE_DEFINITION_FILE", relativePath.getPathName());
				}
			}
		}
		findDefinitions(context, solution, project, (*i)->getItems());
	}
}

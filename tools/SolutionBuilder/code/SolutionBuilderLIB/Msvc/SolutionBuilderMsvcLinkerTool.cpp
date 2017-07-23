/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcLinkerTool.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/File.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"SolutionBuilderMsvcLinkerTool", 1, SolutionBuilderMsvcLinkerTool, SolutionBuilderMsvcTool)

SolutionBuilderMsvcLinkerTool::SolutionBuilderMsvcLinkerTool()
:	m_resolvePaths(false)
,	m_resolveFullLibraryPaths(false)
{
}

bool SolutionBuilderMsvcLinkerTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const
{
	context.set(L"MODULE_DEFINITION_FILE", L"");
	findDefinitions(context, solution, project, project->getItems());

	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCLinkerTool\"" << Endl;

	// Additional options.
	os << L"AdditionalOptions=\"";
	std::map< std::wstring, std::wstring >::const_iterator i0 = m_staticOptions.find(L"AdditionalOptions");
	if (i0 != m_staticOptions.end())
	{
		os << i0->second;
		if (!configuration->getAdditionalLinkerOptions().empty())
			os << L" " << configuration->getAdditionalLinkerOptions();
	}
	else
		os << configuration->getAdditionalLinkerOptions();

	os << L"\"" << Endl;

	// Dependencies.
	os << L"AdditionalDependencies=\"";

	std::set< std::wstring > additionalLibraries;
	std::set< std::wstring > additionalLibraryPaths;

	collectAdditionalLibraries(
		project,
		configuration,
		context.getIncludeExternal(),
		additionalLibraries,
		additionalLibraryPaths
	);

	for (std::set< std::wstring >::const_iterator i = additionalLibraries.begin(); i != additionalLibraries.end(); ++i)
		os << *i << L" ";

	std::map< std::wstring, std::wstring >::const_iterator i1 = m_staticOptions.find(L"AdditionalDependencies");
	if (i1 != m_staticOptions.end())
		os << context.format(i1->second) << L" ";

	os << L"\"" << Endl;

	if (m_staticOptions.find(L"OutputFile") == m_staticOptions.end())
	{
		switch (configuration->getTargetFormat())
		{
		case Configuration::TfSharedLibrary:
			os << L"OutputFile=\"$(OutDir)/" << project->getName() << L".dll\"" << Endl;
			break;

		case Configuration::TfExecutable:
			os << L"OutputFile=\"$(OutDir)/" << project->getName() << L".exe\"" << Endl;
			break;

		default:
			break;
		}
	}

	os << L"AdditionalLibraryDirectories=\"";
	for (std::set< std::wstring >::const_iterator i = additionalLibraryPaths.begin(); i != additionalLibraryPaths.end(); ++i)
	{
		std::wstring libraryPath = context.getProjectRelativePath(*i, m_resolvePaths);
		os << libraryPath << L";";
	}

	std::map< std::wstring, std::wstring >::const_iterator i2 = m_staticOptions.find(L"AdditionalLibraryDirectories");
	if (i2 != m_staticOptions.end())
		os << context.format(i2->second) << L";";

	os << L"\"" << Endl;
	
	if (m_staticOptions.find(L"GenerateDebugInformation") == m_staticOptions.end())
	{
		if (configuration->getTargetProfile() == Configuration::TpDebug)
		{
			os << L"GenerateDebugInformation=\"true\"" << Endl;
			os << L"ProgramDatabaseFile=\"$(OutDir)/" << project->getName() << L".pdb\"" << Endl;
		}
		else
			os << L"GenerateDebugInformation=\"false\"" << Endl;
	}

	if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
	{
		os << L"ImportLibrary=\"$(OutDir)/" << project->getName() << L".lib\"" << Endl;
	}

	// Static options.
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
	{
		if (
			i->first == L"AdditionalOptions" ||
			i->first == L"AdditionalDependencies" ||
			i->first == L"AdditionalLibraryDirectories"
		)
			continue;
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;
	}

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

void SolutionBuilderMsvcLinkerTool::serialize(traktor::ISerializer& s)
{
	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"resolvePaths", m_resolvePaths);
		s >> Member< bool >(L"resolveFullLibraryPaths", m_resolveFullLibraryPaths);
	}
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
}

void SolutionBuilderMsvcLinkerTool::findDefinitions(GeneratorContext& context, Solution* solution, Project* project, const RefArray< ProjectItem >& items) const
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

void SolutionBuilderMsvcLinkerTool::collectAdditionalLibraries(
	Project* project,
	Configuration* configuration,
	bool includeExternal,
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
		// Skip dependencies with we shouldn't link with.
		if ((*i)->getLink() == Dependency::LnkNo)
			continue;

		// Traverse all static library dependencies and add their "additional libraries" as well.
		if (ProjectDependency* projectDependency = dynamic_type_cast< ProjectDependency* >(*i))
		{
			Configuration* dependentConfiguration = projectDependency->getProject()->getConfiguration(configuration->getName());
			if (!dependentConfiguration)
			{
				traktor::log::warning << L"Unable to add dependency \"" << projectDependency->getProject()->getName() << L"\", no matching configuration found" << Endl;
				continue;
			}

			if (dependentConfiguration->getTargetFormat() != Configuration::TfSharedLibrary)
			{
				collectAdditionalLibraries(
					projectDependency->getProject(),
					dependentConfiguration,
					includeExternal,
					outAdditionalLibraries,
					outAdditionalLibraryPaths
				);
			}
		}

		// Add products from external dependencies and add their "additional libraries" as well.
		if (ExternalDependency* externalDependency = dynamic_type_cast< ExternalDependency* >(*i))
		{
			Ref< Configuration > externalConfiguration = externalDependency->getProject()->getConfiguration(configuration->getName());
			if (!externalConfiguration)
			{
				traktor::log::warning << L"Unable to add external dependency \"" << externalDependency->getProject()->getName() << L"\", no matching configuration found" << Endl;
				continue;
			}

			// Add product only if external projects aren't included.
			if (!includeExternal)
			{
				std::wstring externalRootPath = externalDependency->getSolution()->getRootPath();
				std::wstring externalProjectPath = externalRootPath + L"/" + toLower(externalConfiguration->getName());
				std::wstring externalProjectName = externalDependency->getProject()->getName() + L".lib";

				if (!m_resolveFullLibraryPaths)
				{
					outAdditionalLibraries.insert(externalProjectName);
					outAdditionalLibraryPaths.insert(externalProjectPath);
				}
				else
				{
					outAdditionalLibraries.insert(externalProjectPath + L"/" + externalProjectName);
				}
			}

			if (externalConfiguration->getTargetFormat() != Configuration::TfSharedLibrary)
			{
				collectAdditionalLibraries(
					externalDependency->getProject(),
					externalConfiguration,
					includeExternal,
					outAdditionalLibraries,
					outAdditionalLibraryPaths
				);
			}
		}
	}
}

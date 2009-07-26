#include <Core/Io/StringOutputStream.h>
#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberComposite.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Misc/String.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXDefinition.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Configuration.h"

using namespace traktor;

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcVCXDefinition", SolutionBuilderMsvcVCXDefinition, Serializable)

bool SolutionBuilderMsvcVCXDefinition::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	Configuration* configuration,
	OutputStream& os
) const
{
	StringOutputStream ssip, ssd, ssl, sslp;

	const std::vector< std::wstring >& includePaths = configuration->getIncludePaths();
	for (std::vector< std::wstring >::const_iterator i = includePaths.begin(); i != includePaths.end(); ++i)
		ssip << *i << L";";

	const std::vector< std::wstring >& definitions = configuration->getDefinitions();
	for (std::vector< std::wstring >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
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

	os << L"<" << m_name << L">" << Endl;
	os << IncreaseIndent;

	for (std::vector< Option >::const_iterator i = m_options.begin(); i != m_options.end(); ++i)
		os << L"<" << i->name << L">" << context.format(i->value) << L"</" << i->name << L">" << Endl;

	os << DecreaseIndent;
	os << L"</" << m_name << L">" << Endl;

	return true;
}

bool SolutionBuilderMsvcVCXDefinition::serialize(traktor::Serializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"fileTypes", m_fileTypes);
	s >> MemberStlVector< Option, MemberComposite< Option > >(L"options", m_options);
	return true;
}

bool SolutionBuilderMsvcVCXDefinition::Option::serialize(traktor::Serializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< std::wstring >(L"value", value);
	return true;
}

void SolutionBuilderMsvcVCXDefinition::collectAdditionalLibraries(
	Project* project,
	Configuration* configuration,
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

	RefList< Dependency >& dependencies = project->getDependencies();
	for (RefList< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
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
			std::wstring externalProjectName = externalDependency->getProject()->getName() + ((configuration->getTargetProfile() == Configuration::TpDebug) ? L"_d.lib" : L".lib");

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

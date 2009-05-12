#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStl.h>
#include <Core/Serialization/MemberRef.h>
#include <Core/Serialization/MemberStaticArray.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Misc/StringUtils.h>
#include <Core/Misc/MD5.h>
#include <Core/Log/Log.h>
#include <Xml/XmlDeserializer.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvc.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Configuration.h"

using namespace traktor;

namespace
{

	std::wstring generateGuidString(const std::wstring& fileName)
	{
		MD5 md5;
		
		md5.begin();
		md5.feed(fileName.c_str(), int(fileName.length() * sizeof(wchar_t)));
		md5.end();

		const uint8_t* cs = reinterpret_cast< const uint8_t* >(md5.get());

		std::wstringstream ss;
		ss << L"{";
		for (int i = 0; i < 16; ++i)
		{
			wchar_t hex[] =
			{
				L"0123456789ABCDEF"[(cs[i] >> 4) & 0xf],
				L"0123456789ABCDEF"[cs[i] & 0xf]
			};
			ss << hex[0] << hex[1];
			if (i == 3 || i == 5 || i == 7 || i == 9)
				ss << L"-";
		}
		ss << L"}";

		return ss.str();
	}

	std::wstring quotifyString(const std::wstring& str)
	{
		return replaceAll< std::wstring >(str, L"\"", L"&quot;");
	}

	std::wstring getProjectRelativePath(const GeneratorContext& context, const std::wstring& path, bool resolve)
	{
		Path resolvedPath(path);

		// Don't modify absolute paths.
		if (!resolvedPath.isRelative())
			return resolve ? (std::wstring)resolvedPath : path;

		// Transform path into relative to vcproj file.
		Path relativePath;
		FileSystem& fileSystem = FileSystem::getInstance();
		if (fileSystem.getRelativePath(
			fileSystem.getAbsolutePath(path),
			fileSystem.getAbsolutePath(context.get(L"PROJECT_PATH")),
			relativePath
		))
			return relativePath;
		
		return resolve ? (std::wstring)resolvedPath : path;
	}

}

void GeneratorContext::set(const std::wstring& key, const std::wstring& value)
{
	m_values[key] = value;
}

std::wstring GeneratorContext::get(const std::wstring& key) const
{
	std::map< std::wstring, std::wstring >::const_iterator i = m_values.find(key);
	return i != m_values.end() ? i->second : L"";
}

std::wstring GeneratorContext::format(const std::wstring& option) const
{
	std::wstring tmp = option;

	for (;;)
	{
		size_t s = tmp.find(L"%(");
		if (s == std::string::npos)
			break;

		size_t e = tmp.find(L")", s + 2);
		if (e == std::string::npos)
			break;

		std::wstring name = tmp.substr(s + 2, e - s - 2);
		tmp = tmp.substr(0, s) + get(name) + tmp.substr(e + 1);
	}

	return tmp;
}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMsvcTool", SolutionBuilderMsvcTool, Serializable)

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcCompilerTool", SolutionBuilderMsvcCompilerTool, SolutionBuilderMsvcTool)

SolutionBuilderMsvcCompilerTool::SolutionBuilderMsvcCompilerTool()
:	m_resolvePaths(false)
{
}

bool SolutionBuilderMsvcCompilerTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCCLCompilerTool\"" << Endl;

	// Include directories.
	os << L"AdditionalIncludeDirectories=\"";
	for (std::vector< std::wstring >::const_iterator i = configuration->getIncludePaths().begin(); i != configuration->getIncludePaths().end(); ++i)
	{
		std::wstring includePath = getProjectRelativePath(context, *i, m_resolvePaths);
		os << includePath << L";";
	}

	std::map< std::wstring, std::wstring >::const_iterator i1 = m_staticOptions.find(L"AdditionalIncludeDirectories");
	if (i1 != m_staticOptions.end())
		os << context.format(i1->second) << L";";

	os << L"\"" << Endl;

	// Preprocessor definitions.
	os << L"PreprocessorDefinitions=\"";
	for (std::vector< std::wstring >::const_iterator i = configuration->getDefinitions().begin(); i != configuration->getDefinitions().end(); ++i)
		os << *i << L";";

	switch (configuration->getTargetFormat())
	{
	case Configuration::TfStaticLibrary:
		os << L"_LIBRARY;";
		break;

	case Configuration::TfSharedLibrary:
		os << L"_USRDLL;";
		break;
	}

	std::map< std::wstring, std::wstring >::const_iterator i2 = m_staticOptions.find(L"PreprocessorDefinitions");
	if (i2 != m_staticOptions.end())
		os << context.format(i2->second) << L";";

	os << L"\"" << Endl;

	// Precompiled headers.
	std::wstring precompiledHeader = configuration->getPrecompiledHeader();
	if (!precompiledHeader.empty())
	{
		os << L"UsePrecompiledHeader=\"2\"" << Endl;
		os << L"PrecompiledHeaderThrough=\"" << precompiledHeader << L"\"" << Endl;
	}
	else
	{
		os << L"UsePrecompiledHeader=\"0\"" << Endl;
	}

	// Static options.
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
	{
		if (i->first == L"AdditionalIncludeDirectories" || i->first == L"PreprocessorDefinitions")
			continue;
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;
	}

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

bool SolutionBuilderMsvcCompilerTool::serialize(traktor::Serializer& s)
{
	s >> Member< bool >(L"resolvePaths", m_resolvePaths);
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	return true;
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcLibrarianTool", SolutionBuilderMsvcLibrarianTool, SolutionBuilderMsvcTool)

bool SolutionBuilderMsvcLibrarianTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCLibrarianTool\"" << Endl;

	// Output file.
	if (configuration->getTargetProfile() == Configuration::TpDebug)
		os << L"OutputFile=\"$(OutDir)/" << project->getName() << L"_d.lib\"" << Endl;
	else
		os << L"OutputFile=\"$(OutDir)/" << project->getName() << L".lib\"" << Endl;

	// Static options.
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

bool SolutionBuilderMsvcLibrarianTool::serialize(traktor::Serializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	return true;
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcLinkerTool", SolutionBuilderMsvcLinkerTool, SolutionBuilderMsvcTool)

SolutionBuilderMsvcLinkerTool::SolutionBuilderMsvcLinkerTool()
:	m_resolvePaths(false)
{
}

bool SolutionBuilderMsvcLinkerTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const
{
	context.set(L"MODULE_DEFINITION_FILE", L"");
	findDefinitions(context, project, project->getItems());

	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"VCLinkerTool\"" << Endl;

	os << L"AdditionalDependencies=\"";

	std::set< std::wstring > additionalLibraries;
	std::set< std::wstring > additionalLibraryPaths;

	collectAdditionalLibraries(
		project,
		configuration,
		additionalLibraries,
		additionalLibraryPaths
	);

	for (std::set< std::wstring >::const_iterator i = additionalLibraries.begin(); i != additionalLibraries.end(); ++i)
		os << *i << L" ";

	std::map< std::wstring, std::wstring >::const_iterator i1 = m_staticOptions.find(L"AdditionalDependencies");
	if (i1 != m_staticOptions.end())
		os << context.format(i1->second) << L" ";

	os << L"\"" << Endl;

	switch (configuration->getTargetFormat())
	{
	case Configuration::TfSharedLibrary:
		if (configuration->getTargetProfile() == Configuration::TpDebug)
			os << L"OutputFile=\"$(OutDir)/" << project->getName() << L"_d.dll\"" << Endl;
		else
			os << L"OutputFile=\"$(OutDir)/" << project->getName() << L".dll\"" << Endl;
		break;

	case Configuration::TfExecutable:
		if (configuration->getTargetProfile() == Configuration::TpDebug)
			os << L"OutputFile=\"$(OutDir)/" << project->getName() << L"_d.exe\"" << Endl;
		else
			os << L"OutputFile=\"$(OutDir)/" << project->getName() << L".exe\"" << Endl;
		break;
	}

	os << L"AdditionalLibraryDirectories=\"";
	for (std::set< std::wstring >::const_iterator i = additionalLibraryPaths.begin(); i != additionalLibraryPaths.end(); ++i)
	{
		std::wstring libraryPath = getProjectRelativePath(context, *i, m_resolvePaths);
		os << libraryPath << L";";
	}

	std::map< std::wstring, std::wstring >::const_iterator i2 = m_staticOptions.find(L"AdditionalLibraryDirectories");
	if (i2 != m_staticOptions.end())
		os << context.format(i1->second) << L";";

	os << L"\"" << Endl;
	
	std::map< std::wstring, std::wstring >::const_iterator i3 = m_staticOptions.find(L"GenerateDebugInformation");
	if (i3 == m_staticOptions.end())
	{
		if (configuration->getTargetProfile() == Configuration::TpDebug)
		{
			os << L"GenerateDebugInformation=\"true\"" << Endl;
			os << L"ProgramDatabaseFile=\"$(OutDir)/" << project->getName() << L"_d.pdb\"" << Endl;
		}
		else
			os << L"GenerateDebugInformation=\"false\"" << Endl;
	}

	if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
	{
		if (configuration->getTargetProfile() == Configuration::TpDebug)
			os << L"ImportLibrary=\"$(OutDir)/" << project->getName() << L"_d.lib\"" << Endl;
		else
			os << L"ImportLibrary=\"$(OutDir)/" << project->getName() << L".lib\"" << Endl;
	}

	// Static options.
	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
	{
		if (i->first == L"AdditionalDependencies" || i->first == L"AdditionalLibraryDirectories")
			continue;
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;
	}

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

bool SolutionBuilderMsvcLinkerTool::serialize(traktor::Serializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	return true;
}

void SolutionBuilderMsvcLinkerTool::findDefinitions(GeneratorContext& context, Project* project, const RefList< ProjectItem >& items) const
{
	for (RefList< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
	{
		if (const ::File* file = dynamic_type_cast< const ::File* >(*i))
		{
			std::set< Path > systemFiles;
			file->getSystemFiles(project->getSourcePath(), systemFiles);
			for (std::set< Path >::iterator j = systemFiles.begin(); j != systemFiles.end(); ++j)
			{
				if (compareIgnoreCase(Path(j->getFileName()).getExtension(), L"def") == 0)
					context.set(L"MODULE_DEFINITION_FILE", j->getFileName());
			}
		}
		findDefinitions(context, project, (*i)->getItems());
	}
}

void SolutionBuilderMsvcLinkerTool::collectAdditionalLibraries(
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcGenericTool", SolutionBuilderMsvcGenericTool, SolutionBuilderMsvcTool)

bool SolutionBuilderMsvcGenericTool::generate(GeneratorContext& context, Solution* solution, Project* project, Configuration* configuration, traktor::OutputStream& os) const
{
	os << L"<Tool" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"" << m_toolName << L"\"" << Endl;

	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << DecreaseIndent;
	os << L"/>" << Endl;
	return true;
}

bool SolutionBuilderMsvcGenericTool::serialize(traktor::Serializer& s)
{
	s >> Member< std::wstring >(L"toolName", m_toolName);
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	return true;
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcConfiguration", SolutionBuilderMsvcConfiguration, Serializable)

bool SolutionBuilderMsvcConfiguration::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	Configuration* configuration,
	const std::wstring& platform,
	traktor::OutputStream& os
) const
{
	os << L"<Configuration" << Endl;
	os << IncreaseIndent;

	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << L">" << Endl;

	const RefArray< SolutionBuilderMsvcTool >& tools = m_tools[int(configuration->getTargetProfile())];
	for (RefArray< SolutionBuilderMsvcTool >::const_iterator i = tools.begin(); i != tools.end(); ++i)
	{
		if (!(*i)->generate(context, solution, project, configuration, os))
			return false;
	}

	os << DecreaseIndent;
	os << L"</Configuration>" << Endl;

	return true;
}

bool SolutionBuilderMsvcConfiguration::serialize(Serializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	s >> MemberStaticArray< RefArray< SolutionBuilderMsvcTool >, 2, MemberRefArray< SolutionBuilderMsvcTool > >(L"tools", m_tools);
	return true;
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcProject", SolutionBuilderMsvcProject, Serializable)

bool SolutionBuilderMsvcProject::generate(GeneratorContext& context, Solution* solution, Project* project, const std::wstring& projectGuid, traktor::OutputStream& os) const
{
	context.set(L"PLATFORM", m_platform);

	os << L"<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << Endl;
	os << L"<VisualStudioProject" << Endl;
	os << IncreaseIndent;

	for (std::map< std::wstring, std::wstring >::const_iterator i = m_staticOptions.begin(); i != m_staticOptions.end(); ++i)
		os << i->first << L"=\"" << context.format(i->second) << L"\"" << Endl;

	os << L">" << Endl;

	os << L"<Platforms>" << Endl;
	os << IncreaseIndent;
	os << L"<Platform" << Endl;
	os << IncreaseIndent;
	os << L"Name=\"" << m_platform << L"\"" << Endl;
	os << DecreaseIndent;
	os << L"/>" << Endl;
	os << DecreaseIndent;
	os << L"</Platforms>" << Endl;

	os << L"<ToolFiles/>" << Endl;

	os << L"<Configurations>" << Endl;
	os << IncreaseIndent;

	RefList< Configuration >& configurations = project->getConfigurations();
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		context.set(L"CONFIGURATION_NAME", (*i)->getName());

		const SolutionBuilderMsvcConfiguration* configuration = m_configurations[int((*i)->getTargetFormat())];
		if (!configuration->generate(context, solution, project, *i, m_platform, os))
			return false;
	}

	os << DecreaseIndent;
	os << L"</Configurations>" << Endl;

	os << L"<References/>" << Endl;

	os << L"<Files>" << Endl;
	os << IncreaseIndent;

	RefList< ProjectItem >& items = project->getItems();
	for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
	{
		if (!addItem(context, solution, project, *i, os))
			return false;
	}

	os << DecreaseIndent;
	os << L"</Files>" << Endl;

	os << DecreaseIndent;
	os << L"</VisualStudioProject>" << Endl;

	return true;
}

int SolutionBuilderMsvcProject::getVersion() const
{
	return 1;
}

bool SolutionBuilderMsvcProject::serialize(traktor::Serializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"staticOptions", m_staticOptions);
	s >> Member< std::wstring >(L"platform", m_platform);
	s >> MemberStaticArray< Ref< SolutionBuilderMsvcConfiguration >, 4, MemberRef< SolutionBuilderMsvcConfiguration > >(L"configurations", m_configurations);
	
	if (s.getVersion() >= 1)
	{
		s >> MemberStlMap<
			std::wstring,
			Ref< SolutionBuilderMsvcTool >,
			MemberStlPair<
				std::wstring,
				Ref< SolutionBuilderMsvcTool >,
				Member< std::wstring >,
				MemberRef< SolutionBuilderMsvcTool >
			>
		>(L"customTools", m_customTools);
	}

	return true;
}

bool SolutionBuilderMsvcProject::addItem(GeneratorContext& context, Solution* solution, Project* project, ProjectItem* item, OutputStream& os) const
{
	Ref< Filter > filter = dynamic_type_cast< Filter* >(item);
	if (filter)
	{
		os << L"<Filter" << Endl;
		os << IncreaseIndent;
		os << L"Name=\"" << filter->getName() << L"\"" << Endl;
		os << L">" << Endl;

		RefList< ProjectItem >& items = item->getItems();
		for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
			addItem(context, solution, project, *i, os);

		os << DecreaseIndent;
		os << L"</Filter>" << Endl;
	}

	Ref< ::File > file = dynamic_type_cast< ::File* >(item);
	if (file)
	{
		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);

		Path rootPath = FileSystem::getInstance().getAbsolutePath(solution->getRootPath());

		for (std::set< Path >::iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
		{
			Path systemPath = FileSystem::getInstance().getAbsolutePath(*i);

			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				systemPath,
				rootPath,
				relativePath
			);

			std::map< std::wstring, traktor::Ref< SolutionBuilderMsvcTool > >::const_iterator j = m_customTools.find(toLower(relativePath.getExtension()));
			if (j == m_customTools.end())
			{
				// Unable to find any custom build rule; assume it's directly "compilable" by default.
				os << L"<File" << Endl;
				os << IncreaseIndent;
				os << L"RelativePath=\"..\\" << replaceAll< std::wstring >(relativePath, '/', '\\') << L"\"" << Endl;
				os << DecreaseIndent;
				os << L"/>" << Endl;
			}
			else
			{
				// Create custom build rule for this type of file.
				os << L"<File" << Endl;
				os << IncreaseIndent;
				os << L"RelativePath=\"..\\" << replaceAll< std::wstring >(relativePath, '/', '\\') << L"\"" << Endl;
				os << DecreaseIndent;
				os << L">" << Endl;
				os << IncreaseIndent;

				// Add custom configurations to this file.
				RefList< Configuration >& configurations = project->getConfigurations();
				for (RefList< Configuration >::iterator k = configurations.begin(); k != configurations.end(); ++k)
				{
					os << L"<FileConfiguration Name=\"" << (*k)->getName() << L"|" << m_platform << L"\">" << Endl;
					os << IncreaseIndent;

					context.set(L"CONFIGURATION_NAME", (*k)->getName());

					if (!j->second->generate(context, solution, project, *k, os))
						return false;

					os << DecreaseIndent;
					os << L"</FileConfiguration>" << Endl;
				}

				os << DecreaseIndent;
				os << L"</File>" << Endl;
			}
		}
	}

	return true;
}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcSettings", SolutionBuilderMsvcSettings, Serializable)

bool SolutionBuilderMsvcSettings::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"slnVersion", m_slnVersion);
	s >> Member< std::wstring >(L"vsVersion", m_vsVersion);
	s >> MemberRef< SolutionBuilderMsvcProject >(L"project", m_project);
	return true;
}

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMsvc", SolutionBuilderMsvc, SolutionBuilder)

bool SolutionBuilderMsvc::create(const CommandLine& cmdLine)
{
	Ref< SolutionBuilderMsvcSettings > settings = gc_new< SolutionBuilderMsvcSettings >();
	if (cmdLine.hasOption('p'))
	{
		std::wstring platform = cmdLine.getOption('p').getString();

		traktor::log::info << L"Loading settings \"" << platform << L"\"..." << Endl;

		Ref< Stream > file = FileSystem::getInstance().open(platform, traktor::File::FmRead);
		if (!file)
		{
			traktor::log::error << L"Unable to open platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		settings = xml::XmlDeserializer(file).readObject< SolutionBuilderMsvcSettings >();
		if (!settings)
		{
			traktor::log::error << L"Unable to read platform type \"" << platform << L"\"" << Endl;
			return false;
		}

		file->close();
	}
	m_settings = settings;
	return true;
}

bool SolutionBuilderMsvc::generate(Solution* solution)
{
	std::wstring solutionFileName = solution->getRootPath() + L"/" + solution->getName() + L".sln";

	// Setup generator context.
	GeneratorContext context;
	context.set(L"SOLUTION_NAME", solution->getName());
	context.set(L"SOLUTION_ROOTPATH", solution->getRootPath());
	context.set(L"SOLUTION_FILENAME", solutionFileName);

	// Create root path.
	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath()))
		return false;

	// Generate projects.
	std::map< const Project*, std::wstring > projectGuids;

	RefList< Project >& projects = solution->getProjects();
	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Project > project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();
		std::wstring projectFileName = projectPath + L"/" + project->getName() + L".vcproj";
		std::wstring projectGuid = generateGuidString(projectFileName);

		context.set(L"PROJECT_NAME", project->getName());
		context.set(L"PROJECT_PATH", projectPath);
		context.set(L"PROJECT_FILENAME", projectFileName);
		context.set(L"PROJECT_GUID", projectGuid);

		if (!FileSystem::getInstance().makeDirectory(projectPath))
			return false;

		traktor::log::info << L"Generating project \"" << projectFileName << L"\"" << Endl;

		std::vector< uint8_t > buffer;
		buffer.reserve(40000);

		// Generate project file into memory buffer.
		DynamicMemoryStream bufferStream(buffer, false, true);
		FileOutputStream os(&bufferStream, gc_new< AnsiEncoding >());
		bool result = m_settings->getProject()->generate(context, solution, project, projectGuid, os);
		if (!result)
			return false;
		os.close();

		if (!buffer.empty())
		{
			Ref< Stream > file = FileSystem::getInstance().open(
				projectFileName,
				traktor::File::FmWrite
			);
			if (!file)
				return false;
			file->write(&buffer[0], int(buffer.size()));
			file->close();
		}

		projectGuids[project] = projectGuid;
	}

	// Generate solution.
	std::wstring solutionGuid = generateGuidString(solutionFileName);

	Ref< Stream > file = FileSystem::getInstance().open(
		solutionFileName,
		traktor::File::FmWrite
	);
	if (!file)
		return false;

	FileOutputStream os(file, gc_new< AnsiEncoding >());

	os << L"Microsoft Visual Studio Solution File, Format Version " << m_settings->getSLNVersion() << Endl;
	os << L"# Visual Studio " << m_settings->getVSVersion() << Endl;
	
	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Project > project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		os << L"Project(\"" << solutionGuid << L"\") = \"" << project->getName() << L"\", \"" << project->getName() << L"\\" << project->getName() << L".vcproj\", \"" << projectGuids[project] << L"\"" << Endl;

		// Add local dependencies.
		RefList< Dependency >& dependencies = project->getDependencies();
		if (!dependencies.empty())
		{
			os << IncreaseIndent;
			os << L"ProjectSection(ProjectDependencies) = postProject" << Endl;
			for (RefList< Dependency >::iterator j = dependencies.begin(); j != dependencies.end(); ++j)
			{
				if (!is_a< ProjectDependency >(*j))
					continue;

				Ref< Project > dependencyProject = static_cast< ProjectDependency* >(*j)->getProject();
				if (!dependencyProject->getEnable())
				{
					log::warning << L"Trying to add disabled local dependency to project \"" << project->getName() << L"\"; dependency skipped" << Endl;
					continue;
				}

				os << IncreaseIndent;
				os << projectGuids[dependencyProject] << L" = " << projectGuids[dependencyProject] << Endl;
				os << DecreaseIndent;
			}
			os << L"EndProjectSection" << Endl;
			os << DecreaseIndent;
		}

		os << L"EndProject" << Endl;
	}

	os << L"Global" << Endl;
	os << IncreaseIndent;
	os << L"GlobalSection(SolutionConfigurationPlatforms) = preSolution" << Endl;

	std::wstring platform = m_settings->getProject()->getPlatform();

	for (RefList< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		Ref< Project > project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		RefList< Configuration >& configurations = project->getConfigurations();
		for (RefList< Configuration >::iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			Ref< Configuration > configuration = *j;

			os << IncreaseIndent;
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".ActiveCfg = " << configuration->getName() << L"|" << platform << L"" << Endl;
			os << projectGuids[project] << L"." << configuration->getName() << L"|" << platform << L".Build.0 = " << configuration->getName() << L"|" << platform << L"" << Endl;
			os << DecreaseIndent;
		}
	}

	os << L"EndGlobalSection" << Endl;
	os << DecreaseIndent;

	os << IncreaseIndent;
	os << L"GlobalSection(SolutionProperties) = preSolution" << Endl;
	os << IncreaseIndent;
	os << L"HideSolutionNode = FALSE" << Endl;
	os << DecreaseIndent;
	os << L"EndGlobalSection" << Endl;
	os << DecreaseIndent;
	os << L"EndGlobal" << Endl;

	os.close();

	return true;
}

void SolutionBuilderMsvc::showOptions() const
{
	traktor::log::info << L"\t-p=[platform]	(Platform description file)" << Endl;
}

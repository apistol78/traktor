#include <Core/Io/FileSystem.h>
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberStaticArray.h>
#include <Core/Serialization/MemberRef.h>
#include <Core/Misc/String.h>
#include <Core/Misc/MD5.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXProj.h"
#include "SolutionBuilderLIB/Msvc/SolutionBuilderMsvcVCXDefinition.h"
#include "SolutionBuilderLIB/Msvc/GeneratorContext.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/File.h"

using namespace traktor;

namespace
{

	std::wstring systemPath(const Path& path)
	{
		return replaceAll< std::wstring >(path, L'/', L'\\');
	}

}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"SolutionBuilderMsvcVCXProj", SolutionBuilderMsvcVCXProj, SolutionBuilderMsvcProject)

std::wstring SolutionBuilderMsvcVCXProj::getPlatform() const
{
	return m_platform;
}

bool SolutionBuilderMsvcVCXProj::getInformation(
	GeneratorContext& context,
	Solution* solution,
	Project* project,
	std::wstring& outProjectPath,
	std::wstring& outProjectFileName,
	std::wstring& outProjectGuid
) const
{
	outProjectPath = solution->getRootPath() + L"/" + project->getName();
	outProjectFileName = outProjectPath + L"/" + project->getName() + L".vcxproj";
	outProjectGuid = context.generateGUID(outProjectFileName);
	return true;
}

bool SolutionBuilderMsvcVCXProj::generate(
	GeneratorContext& context,
	Solution* solution,
	Project* project
) const
{
	std::wstring projectPath, projectFileName, projectGuid;
	if (!getInformation(context, solution, project, projectPath, projectFileName, projectGuid))
		return false;

	if (!FileSystem::getInstance().makeDirectory(projectPath))
		return false;

	traktor::log::info << L"Generating msbuild project \"" << projectFileName << L"\"" << Endl;

	context.set(L"PROJECT_PLATFORM", m_platform);
	context.set(L"PROJECT_NAME", project->getName());
	context.set(L"PROJECT_PATH", projectPath);
	context.set(L"PROJECT_FILENAME", projectFileName);
	context.set(L"PROJECT_GUID", projectGuid);

	std::vector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, gc_new< AnsiEncoding >());

	os << L"<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << Endl;
	os << IncreaseIndent;

	// Configurations
	os << L"<ItemGroup Label=\"ProjectConfigurations\">" << Endl;
	os << IncreaseIndent;

	RefList< Configuration >& configurations = project->getConfigurations();
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		Configuration* configuration = *i;

		os << L"<ProjectConfiguration Include=\"" << configuration->getName() << L"|" << m_platform << L"\">" << Endl;
		os << IncreaseIndent;

		os << L"<Configuration>" << configuration->getName() << L"</Configuration>" << Endl;
		os << L"<Platform>" << m_platform << L"</Platform>" << Endl;

		os << DecreaseIndent;
		os << L"</ProjectConfiguration>" << Endl;
	}

	os << DecreaseIndent;
	os << L"</ItemGroup>" << Endl;

	// Globals
	os << L"<PropertyGroup Label=\"Globals\">" << Endl;
	os << IncreaseIndent;
	os << L"<Keyword>" << m_keyword << L"</Keyword>" << Endl;
	os << L"<ProjectGUID>" << projectGuid << L"</ProjectGUID>" << Endl;
	os << L"<RootNamespace>" << project->getName() << L"</RootNamespace>" << Endl;
	os << DecreaseIndent;
	os << L"</PropertyGroup>" << Endl;

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />" << Endl;

	// Configurations
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		Configuration* configuration = *i;

		os << L"<PropertyGroup Label=\"Configuration\" Condition=\"'$(Configuration)|$(Platform)'=='" << configuration->getName() << L"|" << m_platform << L"'\">" << Endl;
		os << IncreaseIndent;
		os << L"<CharacterSet>MultiByte</CharacterSet>" << Endl;

		switch (configuration->getTargetFormat())
		{
		case Configuration::TfStaticLibrary:
			os << L"<ConfigurationType>StaticLibrary</ConfigurationType>" << Endl;
			break;

		case Configuration::TfSharedLibrary:
			os << L"<ConfigurationType>DynamicLibrary</ConfigurationType>" << Endl;
			break;

		case Configuration::TfExecutable:
		case Configuration::TfExecutableConsole:
			os << L"<ConfigurationType>Application</ConfigurationType>" << Endl;
			break;
		}
		
		os << L"<WholeProgramOptimization>false</WholeProgramOptimization>" << Endl;
		os << DecreaseIndent;
		os << L"</PropertyGroup>" << Endl;
	}

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />" << Endl;
	os << L"<ImportGroup Label=\"PropertySheets\">" << Endl;
	os << IncreaseIndent;
	os << L"<Import Project=\"$(LocalAppData)\\Microsoft\\VisualStudio\\10.0\\Microsoft.Cpp.$(Platform).user.props\" Condition=\"exists('$(LocalAppData)\\Microsoft\\VisualStudio\\10.0\\Microsoft.Cpp.$(Platform).user.props')\" />" << Endl;
	os << DecreaseIndent;
	os << L"</ImportGroup>" << Endl;

	// Properties
	os << L"<PropertyGroup>" << Endl;
	os << IncreaseIndent;
	os << L"<_ProjectFileVersion>10.0.20506.1</_ProjectFileVersion>" << Endl;
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		Configuration* configuration = *i;
		std::wstring name = configuration->getName();

		std::wstring projectName = project->getName();
		if (configuration->getTargetProfile() == Configuration::TpDebug)
			projectName += L"_d";

		os << L"<IntDir Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">$(Configuration)\\</IntDir>" << Endl;
		os << L"<OutDir Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">$(SolutionDir)$(Configuration)\\</OutDir>" << Endl;
		os << L"<TargetName Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">" << projectName << L"</TargetName>" << Endl;

		switch (configuration->getTargetFormat())
		{
		case Configuration::TfStaticLibrary:
			os << L"<TargetExt Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">.lib</TargetExt>" << Endl;
			break;

		case Configuration::TfSharedLibrary:
			os << L"<TargetExt Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">.dll</TargetExt>" << Endl;
			if (configuration->getTargetProfile() == Configuration::TpRelease)
				os << L"<LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">false</LinkIncremental>" << Endl;
			else
				os << L"<LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">true</LinkIncremental>" << Endl;
			os << L"<EmbedManifest Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">true</EmbedManifest>" << Endl;
			break;

		case Configuration::TfExecutable:
		case Configuration::TfExecutableConsole:
			os << L"<TargetExt Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">.exe</TargetExt>" << Endl;
			if (configuration->getTargetProfile() == Configuration::TpRelease)
				os << L"<LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">false</LinkIncremental>" << Endl;
			else
				os << L"<LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">true</LinkIncremental>" << Endl;
			os << L"<EmbedManifest Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">true</EmbedManifest>" << Endl;
			break;
		}
	}
	os << DecreaseIndent;
	os << L"</PropertyGroup>" << Endl;

	// Build definitions.
	for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
	{
		Configuration* configuration = *i;
		std::wstring name = configuration->getName();

		os << L"<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">" << Endl;
		os << IncreaseIndent;

		int32_t format = int32_t(configuration->getTargetFormat());

		const RefArray< SolutionBuilderMsvcVCXDefinition >& buildDefinitions = (configuration->getTargetProfile() == Configuration::TpDebug) ? m_buildDefinitionsDebug[format] : m_buildDefinitionsRelease[format];
		for (RefArray< SolutionBuilderMsvcVCXDefinition >::const_iterator j = buildDefinitions.begin(); j != buildDefinitions.end(); ++j)
		{
			(*j)->generate(
				context,
				solution,
				project,
				configuration,
				os
			);
		}

		os << DecreaseIndent;
		os << L"</ItemDefinitionGroup>" << Endl;
	}

	// Collect all files.
	std::vector< Path > files;
	RefList< ProjectItem >& items = project->getItems();
	for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
	{
		if (!collectFiles(project, *i, files))
			return false;
	}

	// Create custom build item group(s).
	for (std::vector< Path >::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		if (
			compareIgnoreCase(i->getExtension(), L"png") == 0 ||
			compareIgnoreCase(i->getExtension(), L"xdi") == 0
		)
		{
			os << L"<ItemGroup>" << Endl;
			os << IncreaseIndent;
			os << L"<CustomBuild Include=\"" << systemPath(*i) << L"\">" << Endl;
			os << IncreaseIndent;

			for (RefList< Configuration >::iterator i = configurations.begin(); i != configurations.end(); ++i)
			{
				Configuration* configuration = *i;
				std::wstring name = configuration->getName();

				os << L"<Command Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">$(TRAKTOR_HOME)/bin/BinaryInclude %(FullPath) .\\Resources\\%(FileName).h c_Resource%(FileName)</Command>" << Endl;
				os << L"<Message Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">Building embedded resource file %(FileName).h</Message>" << Endl;
				os << L"<Outputs Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">.\\Resources\\%(FileName).h;%(Outputs)</Outputs>" << Endl;
			}

			os << DecreaseIndent;
			os << L"</CustomBuild>" << Endl;
			os << DecreaseIndent;
			os << L"</ItemGroup>" << Endl;
		}
	}

	// Create item groups.
	const wchar_t* itemGroups[7][2] =
	{
		{ L"ClInclude", L"hpp" },
		{ L"ClInclude", L"h" },
		{ L"ClCompile", L"cpp" },
		{ L"ClCompile", L"cc" },
		{ L"ClCompile", L"c" },
		{ L"ResourceCompile", L"rc" },
		{ L"None", L"inl" }
	};
	for (int i = 0; i < sizeof_array(itemGroups); ++i)
	{
		os << L"<ItemGroup>" << Endl;
		os << IncreaseIndent;

		for (std::vector< Path >::const_iterator j = files.begin(); j != files.end(); ++j)
		{
			if (compareIgnoreCase(j->getExtension(), itemGroups[i][1]) == 0)
			{
				Path itemPath;
				FileSystem::getInstance().getRelativePath(
					FileSystem::getInstance().getAbsolutePath(*j),
					FileSystem::getInstance().getAbsolutePath(projectPath),
					itemPath
				);
				os << L"<" << itemGroups[i][0] << L" Include=\"" << systemPath(itemPath.getPathName()) << L"\" />" << Endl;
			}
		}

		os << DecreaseIndent;
		os << L"</ItemGroup>" << Endl;
	}

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />" << Endl;

	os << L"<ItemGroup>" << Endl;
	os << IncreaseIndent;

	const RefList< Dependency >& dependencies = project->getDependencies();
	for (RefList< Dependency >::const_iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		Ref< const ProjectDependency > projectDependency = dynamic_type_cast< const ProjectDependency* >(*i);
		if (projectDependency)
		{
			Ref< Project > dependentProject = projectDependency->getProject();
			T_ASSERT (dependentProject);

			std::wstring dependentProjectPath;
			std::wstring dependentProjectFileName;
			std::wstring dependentProjectGuid;

			if (!getInformation(
				context,
				solution,
				dependentProject,
				dependentProjectPath,
				dependentProjectFileName,
				dependentProjectGuid
			))
				return false;

			Path relativePath;
			FileSystem::getInstance().getRelativePath(
				dependentProjectFileName,
				projectPath,
				relativePath
			);

			os << L"<ProjectReference Include=\"" << systemPath(relativePath.getPathName()) << L"\">" << Endl;
			os << IncreaseIndent;
			os << L"<Project>" << dependentProjectGuid << L"</Project>" << Endl;
			os << DecreaseIndent;
			os << L"</ProjectReference>" << Endl;
		}
	}

	os << DecreaseIndent;
	os << L"</ItemGroup>" << Endl;

	os << DecreaseIndent;
	os << L"</Project>" << Endl;

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

	return true;
}

bool SolutionBuilderMsvcVCXProj::serialize(traktor::Serializer& s)
{
	s >> Member< std::wstring >(L"platform", m_platform);
	s >> Member< std::wstring >(L"keyword", m_keyword);
	s >> MemberStaticArray<
			RefArray< SolutionBuilderMsvcVCXDefinition >,
			sizeof_array(m_buildDefinitionsDebug),
			MemberRefArray< SolutionBuilderMsvcVCXDefinition >
		>(L"buildDefinitionsDebug", m_buildDefinitionsDebug);
	s >> MemberStaticArray<
			RefArray< SolutionBuilderMsvcVCXDefinition >,
			sizeof_array(m_buildDefinitionsRelease),
			MemberRefArray< SolutionBuilderMsvcVCXDefinition >
		>(L"buildDefinitionsRelease", m_buildDefinitionsRelease);
	return true;
}

bool SolutionBuilderMsvcVCXProj::collectFiles(
	Project* project,
	ProjectItem* item,
	std::vector< Path >& outFiles
) const
{
	Ref< Filter > filter = dynamic_type_cast< Filter* >(item);
	if (filter)
	{
		RefList< ProjectItem >& items = item->getItems();
		for (RefList< ProjectItem >::iterator i = items.begin(); i != items.end(); ++i)
			collectFiles(
				project,
				*i,
				outFiles
			);
	}

	Ref< ::File > file = dynamic_type_cast< ::File* >(item);
	if (file)
	{
		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);
		for (std::set< Path >::iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
			outFiles.push_back(*i);
	}

	return true;
}

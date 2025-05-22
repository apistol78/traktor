/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXProj.h"

#include "Core/Guid.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/System/ResolveEnv.h"
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/File.h"
#include "SolutionBuilder/Filter.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXBuildTool.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXDefinition.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImport.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImportGroup.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXPropertyGroup.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Utilities.h"

namespace traktor::sb
{
namespace
{

std::wstring systemPath(const Path& path)
{
	return replaceAll(path.getPathName(), L'/', L'\\');
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.SolutionBuilderMsvcVCXProj", 10, SolutionBuilderMsvcVCXProj, ISerializable)

SolutionBuilderMsvcVCXProj::SolutionBuilderMsvcVCXProj()
	: m_resolvePaths(true)
{
	m_targetPrefixes[0] = L"";
	m_targetPrefixes[1] = L"";
	m_targetPrefixes[2] = L"";
	m_targetPrefixes[3] = L"";
	m_targetExts[0] = L".lib";
	m_targetExts[1] = L".dll";
	m_targetExts[2] = L".exe";
	m_targetExts[3] = L".exe";
}

std::wstring SolutionBuilderMsvcVCXProj::getPlatform() const
{
	return m_platform;
}

bool SolutionBuilderMsvcVCXProj::getInformation(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	std::wstring& outSolutionPath,
	std::wstring& outProjectPath,
	std::wstring& outProjectFileName,
	std::wstring& outProjectGuid) const
{
	outSolutionPath = solution->getRootPath();
	outProjectPath = solution->getRootPath() + L"/" + project->getName();
	outProjectFileName = outProjectPath + L"/" + project->getName() + L".vcxproj";
	outProjectGuid = context.generateGUID(outProjectFileName);
	return true;
}

bool SolutionBuilderMsvcVCXProj::generate(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project) const
{
	if (!generateProject(context, solution, project))
		return false;

	if (!generateFilters(context, solution, project))
		return false;

	if (!generateUser(context, solution, project))
		return false;

	return true;
}

void SolutionBuilderMsvcVCXProj::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 9);

	const wchar_t* itemNames[] = { L"staticLibrary", L"sharedLibrary", L"executable", L"executableConsole" };

	s >> Member< std::wstring >(L"platform", m_platform);
	s >> Member< std::wstring >(L"keyword", m_keyword);
	s >> Member< std::wstring >(L"windowsTargetPlatformVersion", m_windowsTargetPlatformVersion);
	s >> MemberStaticArray< std::wstring, sizeof_array(m_targetPrefixes) >(L"targetPrefixes", m_targetPrefixes);
	s >> MemberStaticArray< std::wstring, sizeof_array(m_targetExts) >(L"targetExts", m_targetExts);
	s >> Member< bool >(L"resolvePaths", m_resolvePaths);
	s >> MemberRefArray< SolutionBuilderMsvcVCXPropertyGroup >(L"propertyGroupsBeforeImports", m_propertyGroupsBeforeImports);
	s >> MemberRefArray< SolutionBuilderMsvcVCXImportCommon >(L"imports", m_imports);
	s >> MemberRefArray< SolutionBuilderMsvcVCXPropertyGroup >(L"propertyGroupsAfterImports", m_propertyGroupsAfterImports);

	if (s.getVersion< SolutionBuilderMsvcVCXProj >() >= 10)
	{
		s >> MemberComposite< Profile >(L"profileDebug", m_profileDebug);
		s >> MemberComposite< Profile >(L"profileRelease", m_profileRelease);
	}
	else
	{
		std::map< std::wstring, std::wstring > m_configurationDefinitionsDebug[4];
		std::map< std::wstring, std::wstring > m_configurationDefinitionsRelease[4];
		RefArray< SolutionBuilderMsvcVCXDefinition > m_buildDefinitionsDebug[4];
		RefArray< SolutionBuilderMsvcVCXDefinition > m_buildDefinitionsRelease[4];

		s >> MemberStaticArray<
				 std::map< std::wstring, std::wstring >,
				 4,
				 MemberStlMap< std::wstring, std::wstring > >(L"configurationDefinitionsDebug", m_configurationDefinitionsDebug, itemNames);
		s >> MemberStaticArray<
				 std::map< std::wstring, std::wstring >,
				 4,
				 MemberStlMap< std::wstring, std::wstring > >(L"configurationDefinitionsRelease", m_configurationDefinitionsRelease, itemNames);
		s >> MemberStaticArray<
				 RefArray< SolutionBuilderMsvcVCXDefinition >,
				 sizeof_array(m_buildDefinitionsDebug),
				 MemberRefArray< SolutionBuilderMsvcVCXDefinition > >(L"buildDefinitionsDebug", m_buildDefinitionsDebug, itemNames);
		s >> MemberStaticArray<
				 RefArray< SolutionBuilderMsvcVCXDefinition >,
				 sizeof_array(m_buildDefinitionsRelease),
				 MemberRefArray< SolutionBuilderMsvcVCXDefinition > >(L"buildDefinitionsRelease", m_buildDefinitionsRelease, itemNames);

		m_profileDebug.staticLibrary.configurationDefinitions = m_configurationDefinitionsDebug[0];
		m_profileDebug.staticLibrary.buildDefinitions = m_buildDefinitionsDebug[0];
		m_profileDebug.sharedLibrary.configurationDefinitions = m_configurationDefinitionsDebug[1];
		m_profileDebug.sharedLibrary.buildDefinitions = m_buildDefinitionsDebug[1];
		m_profileDebug.executable.configurationDefinitions = m_configurationDefinitionsDebug[2];
		m_profileDebug.executable.buildDefinitions = m_buildDefinitionsDebug[2];
		m_profileDebug.executableConsole.configurationDefinitions = m_configurationDefinitionsDebug[3];
		m_profileDebug.executableConsole.buildDefinitions = m_buildDefinitionsDebug[3];

		m_profileRelease.staticLibrary.configurationDefinitions = m_configurationDefinitionsRelease[0];
		m_profileRelease.staticLibrary.buildDefinitions = m_buildDefinitionsRelease[0];
		m_profileRelease.sharedLibrary.configurationDefinitions = m_configurationDefinitionsRelease[1];
		m_profileRelease.sharedLibrary.buildDefinitions = m_buildDefinitionsRelease[1];
		m_profileRelease.executable.configurationDefinitions = m_configurationDefinitionsRelease[2];
		m_profileRelease.executable.buildDefinitions = m_buildDefinitionsRelease[2];
		m_profileRelease.executableConsole.configurationDefinitions = m_configurationDefinitionsRelease[3];
		m_profileRelease.executableConsole.buildDefinitions = m_buildDefinitionsRelease[3];
	}

	s >> MemberRefArray< SolutionBuilderMsvcVCXBuildTool >(L"buildTools", m_buildTools);
}

void SolutionBuilderMsvcVCXProj::Product::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"configurationDefinitions", configurationDefinitions);
	s >> MemberRefArray< SolutionBuilderMsvcVCXDefinition >(L"buildDefinitions", buildDefinitions);
}

const SolutionBuilderMsvcVCXProj::Product& SolutionBuilderMsvcVCXProj::Profile::getProduct(Configuration::TargetFormat targetFormat) const
{
	switch (targetFormat)
	{
	case Configuration::TfStaticLibrary:
		return staticLibrary;
	case Configuration::TfSharedLibrary:
		return sharedLibrary;
	case Configuration::TfExecutable:
		return executable;
	case Configuration::TfExecutableConsole:
		return executableConsole;
	}
	T_FATAL_ASSERT(false);
	return staticLibrary;
}

void SolutionBuilderMsvcVCXProj::Profile::serialize(ISerializer& s)
{
	s >> MemberComposite< Product >(L"staticLibrary", staticLibrary);
	s >> MemberComposite< Product >(L"sharedLibrary", sharedLibrary);
	s >> MemberComposite< Product >(L"executable", executable);
	s >> MemberComposite< Product >(L"executableConsole", executableConsole);
}

bool SolutionBuilderMsvcVCXProj::generateProject(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project) const
{
	std::wstring solutionPath, projectPath, projectFileName, projectGuid;
	if (!getInformation(
			context,
			solution,
			project,
			solutionPath,
			projectPath,
			projectFileName,
			projectGuid))
		return false;

	if (!FileSystem::getInstance().makeAllDirectories(projectPath))
		return false;

	log::info << L"Generating msbuild project \"" << projectFileName << L"\"" << Endl;

	context.set(L"SOLUTION_PATH", solutionPath);
	context.set(L"PROJECT_PLATFORM", m_platform);
	context.set(L"PROJECT_NAME", project->getName());
	context.set(L"PROJECT_PATH", projectPath);
	context.set(L"PROJECT_FILENAME", projectFileName);
	context.set(L"PROJECT_GUID", projectGuid);

	context.set(L"MODULE_DEFINITION_FILE", L"");
	findDefinitions(context, solution, project, project->getItems());

	AlignedVector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new AnsiEncoding());

	os << L"<Project DefaultTargets=\"Build\" ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << Endl;
	os << IncreaseIndent;

	// Custom property groups.
	for (auto propertyGroup : m_propertyGroupsBeforeImports)
		propertyGroup->generate(
			context,
			solution,
			project,
			os);

	// Configurations
	os << L"<ItemGroup Label=\"ProjectConfigurations\">" << Endl;
	os << IncreaseIndent;

	for (auto configuration : project->getConfigurations())
	{
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

	if (!m_keyword.empty())
		os << L"<Keyword>" << m_keyword << L"</Keyword>" << Endl;

	os << L"<ProjectGUID>" << projectGuid << L"</ProjectGUID>" << Endl;
	os << L"<RootNamespace>" << project->getName() << L"</RootNamespace>" << Endl;

	if (!m_windowsTargetPlatformVersion.empty())
		os << L"<WindowsTargetPlatformVersion>" << m_windowsTargetPlatformVersion << L"</WindowsTargetPlatformVersion>" << Endl;

	os << DecreaseIndent;
	os << L"</PropertyGroup>" << Endl;

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />" << Endl;

	// Configurations
	for (auto configuration : project->getConfigurations())
	{
		os << L"<PropertyGroup Label=\"Configuration\" Condition=\"'$(Configuration)|$(Platform)'=='" << configuration->getName() << L"|" << m_platform << L"'\">" << Endl;
		os << IncreaseIndent;

		if (configuration->getTargetProfile() == Configuration::TpDebug)
		{
			const auto& cd = m_profileDebug.getProduct(configuration->getTargetFormat()).configurationDefinitions;
			for (auto i = cd.begin(); i != cd.end(); ++i)
				os << L"<" << i->first << L">" << i->second << L"</" << i->first << L">" << Endl;
		}
		else
		{
			const auto& cd = m_profileRelease.getProduct(configuration->getTargetFormat()).configurationDefinitions;
			for (auto i = cd.begin(); i != cd.end(); ++i)
				os << L"<" << i->first << L">" << i->second << L"</" << i->first << L">" << Endl;
		}

		os << DecreaseIndent;
		os << L"</PropertyGroup>" << Endl;
	}

	// Imports
	for (auto import : m_imports)
		import->generate(os);

	// Custom property groups.
	for (auto propertyGroup : m_propertyGroupsAfterImports)
		propertyGroup->generate(
			context,
			solution,
			project,
			os);

	// Properties
	os << L"<PropertyGroup>" << Endl;
	os << IncreaseIndent;
	os << L"<_ProjectFileVersion>10.0.20506.1</_ProjectFileVersion>" << Endl;
	for (auto configuration : project->getConfigurations())
	{
		std::wstring name = configuration->getName();
		std::wstring projectName = m_targetPrefixes[int(configuration->getTargetFormat())] + project->getName();

		os << L"<IntDir Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">$(Configuration)\\</IntDir>" << Endl;
		os << L"<OutDir Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">$(ProjectDir)..\\$(Configuration)\\</OutDir>" << Endl;
		os << L"<TargetName Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">" << projectName << L"</TargetName>" << Endl;
		os << L"<TargetExt Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">" << m_targetExts[int(configuration->getTargetFormat())] << L"</TargetExt>" << Endl;

		switch (configuration->getTargetFormat())
		{
		case Configuration::TfStaticLibrary:
			break;

		case Configuration::TfSharedLibrary:
			if (configuration->getTargetProfile() == Configuration::TpRelease)
				os << L"<LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">false</LinkIncremental>" << Endl;
			else
				os << L"<LinkIncremental Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">true</LinkIncremental>" << Endl;
			os << L"<EmbedManifest Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">true</EmbedManifest>" << Endl;
			break;

		case Configuration::TfExecutable:
		case Configuration::TfExecutableConsole:
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
	for (auto configuration : project->getConfigurations())
	{
		const std::wstring name = configuration->getName();

		os << L"<ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='" << name << L"|" << m_platform << L"'\">" << Endl;
		os << IncreaseIndent;

		const Profile& profile = (configuration->getTargetProfile() == Configuration::TpDebug) ? m_profileDebug : m_profileRelease;
		const auto& buildDefinitions = profile.getProduct(configuration->getTargetFormat()).buildDefinitions;

		for (auto buildDefinition : buildDefinitions)
			buildDefinition->generate(
				context,
				solution,
				project,
				configuration,
				os);

		const auto& aggregationItems = configuration->getAggregationItems();
		if (!aggregationItems.empty())
		{
			os << L"<PostBuildEvent>" << Endl;
			os << IncreaseIndent;
			os << L"<Command>";

			const int32_t indent = os.getIndent();
			os.setIndent(0);

			const Path aggregateOutputPath = FileSystem::getInstance().getAbsolutePath(solution->getAggregateOutputPath());

			os << L"@pushd \"$(SolutionDir)" << name << L"\"" << Endl;
			for (auto aggregationItem : aggregationItems)
			{
				const Path sourceFile = Path(aggregationItem->getSourceFile());
				const Path targetPath = aggregateOutputPath + Path(aggregationItem->getTargetPath());
				os << L"@xcopy /D /F /R /Y /I \"" << sourceFile.getPathName() << L"\" \"" << targetPath.getPathName() << L"\\\"" << Endl;
			}
			os << L"@popd" << Endl;

			os.setIndent(indent);
			os << L"</Command>" << Endl;
			os << DecreaseIndent;
			os << L"</PostBuildEvent>" << Endl;
		}

		os << DecreaseIndent;
		os << L"</ItemDefinitionGroup>" << Endl;
	}

	// Collect all files.
	std::vector< std::pair< std::wstring, Path > > files;
	for (auto item : project->getItems())
		if (!collectFiles(project, item, L"", files))
			return false;

	// Create item groups.
	for (auto buildTool : m_buildTools)
	{
		os << L"<ItemGroup>" << Endl;
		os << IncreaseIndent;

		for (const auto& file : files)
		{
			Path itemPath;
			FileSystem::getInstance().getRelativePath(
				FileSystem::getInstance().getAbsolutePath(file.second),
				FileSystem::getInstance().getAbsolutePath(projectPath),
				itemPath);
			buildTool->generateProject(
				context,
				solution,
				project,
				file.first,
				itemPath,
				os);
		}

		os << DecreaseIndent;
		os << L"</ItemGroup>" << Endl;
	}

	os << L"<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\" />" << Endl;

	// Dependencies.
	os << L"<ItemGroup>" << Endl;
	os << IncreaseIndent;

	for (auto dependency : project->getDependencies())
	{
		auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency);
		if (projectDependency)
		{
			auto dependentProject = projectDependency->getProject();
			T_ASSERT(dependentProject);

			std::wstring dependentSolutionPath;
			std::wstring dependentProjectPath;
			std::wstring dependentProjectFileName;
			std::wstring dependentProjectGuid;

			if (!getInformation(
					context,
					solution,
					dependentProject,
					dependentSolutionPath,
					dependentProjectPath,
					dependentProjectFileName,
					dependentProjectGuid))
				return false;

			Path relativePath;
			if (!FileSystem::getInstance().getRelativePath(
					FileSystem::getInstance().getAbsolutePath(dependentProjectFileName),
					FileSystem::getInstance().getAbsolutePath(projectPath),
					relativePath))
			{
				log::warning << L"Unable to determine relative path to \"" << dependentProjectFileName << L"\" from \"" << projectPath << L"\"." << Endl;
				continue;
			}

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

	if (!writeFileIfMismatch(projectFileName, buffer))
		return false;

	return true;
}

bool SolutionBuilderMsvcVCXProj::generateFilters(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project) const
{
	std::wstring solutionPath, projectPath, projectFileName, projectGuid;
	if (!getInformation(
			context,
			solution,
			project,
			solutionPath,
			projectPath,
			projectFileName,
			projectGuid))
		return false;

	if (!FileSystem::getInstance().makeAllDirectories(projectPath))
		return false;

	log::info << L"Generating msbuild filters \"" << projectFileName << L".filters\"" << Endl;

	context.set(L"PROJECT_PLATFORM", m_platform);
	context.set(L"PROJECT_NAME", project->getName());
	context.set(L"PROJECT_PATH", projectPath);
	context.set(L"PROJECT_FILENAME", projectFileName);
	context.set(L"PROJECT_GUID", projectGuid);

	AlignedVector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new AnsiEncoding());

	os << L"<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << Endl;
	os << IncreaseIndent;

	// Collect all files.
	std::vector< std::pair< std::wstring, Path > > files;
	for (auto item : project->getItems())
		if (!collectFiles(project, item, L"", files))
			return false;

	// Create item groups.
	for (auto buildTool : m_buildTools)
	{
		os << L"<ItemGroup>" << Endl;
		os << IncreaseIndent;

		for (const auto& file : files)
		{
			Path itemPath;
			FileSystem::getInstance().getRelativePath(
				FileSystem::getInstance().getAbsolutePath(file.second),
				FileSystem::getInstance().getAbsolutePath(projectPath),
				itemPath);
			buildTool->generateFilter(
				context,
				solution,
				project,
				file.first,
				itemPath,
				os);
		}

		os << DecreaseIndent;
		os << L"</ItemGroup>" << Endl;
	}

	// Create filter guids.
	Guid filterGuid(L"{4708E59F-6655-4B59-A318-ECFC32369845}");

	std::set< std::wstring > filters;
	for (const auto& file : files)
		if (file.first != L"")
			filters.insert(file.first);

	if (!filters.empty())
	{
		for (const auto& filter : filters)
		{
			os << L"<ItemGroup>" << Endl;
			os << IncreaseIndent;

			os << L"<Filter Include=\"" << filter << L"\">" << Endl;
			os << L"<UniqueIdentifier>" << filterGuid.format() << L"</UniqueIdentifier>" << Endl;
			os << L"</Filter>" << Endl;

			os << DecreaseIndent;
			os << L"</ItemGroup>" << Endl;

			filterGuid = filterGuid.permutation(1);
		}
	}

	os << DecreaseIndent;
	os << L"</Project>" << Endl;

	os.close();

	if (!writeFileIfMismatch(projectFileName + L".filters", buffer))
		return false;

	return true;
}

bool SolutionBuilderMsvcVCXProj::generateUser(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project) const
{
	std::wstring solutionPath, projectPath, projectFileName, projectGuid;
	if (!getInformation(
			context,
			solution,
			project,
			solutionPath,
			projectPath,
			projectFileName,
			projectGuid))
		return false;

	if (!FileSystem::getInstance().makeAllDirectories(projectPath))
		return false;

	log::info << L"Generating msbuild user \"" << projectFileName << L".user\"" << Endl;

	context.set(L"PROJECT_PLATFORM", m_platform);
	context.set(L"PROJECT_NAME", project->getName());
	context.set(L"PROJECT_PATH", projectPath);
	context.set(L"PROJECT_FILENAME", projectFileName);
	context.set(L"PROJECT_GUID", projectGuid);

	AlignedVector< uint8_t > buffer;
	buffer.reserve(40000);

	DynamicMemoryStream bufferStream(buffer, false, true);
	FileOutputStream os(&bufferStream, new AnsiEncoding());

	bool needed = false;

	os << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>" << Endl;
	os << L"<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">" << Endl;
	os << IncreaseIndent;

	for (auto configuration : project->getConfigurations())
	{
		os << L"<PropertyGroup Condition=\"'$(Configuration)'=='" << configuration->getName() << L"'\">" << Endl;
		os << IncreaseIndent;

		os << L"<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>" << Endl;

		if (!configuration->getDebugEnvironment().empty())
		{
			os << L"<LocalDebuggerEnvironment>" << resolveEnv(configuration->getDebugEnvironment(), nullptr) << L"</LocalDebuggerEnvironment>" << Endl;
			needed = true;
		}

		if (!configuration->getDebugExecutable().empty())
		{
			os << L"<LocalDebuggerCommand>" << resolveEnv(configuration->getDebugExecutable(), nullptr) << L"</LocalDebuggerCommand>" << Endl;
			needed = true;
		}

		if (!configuration->getDebugWorkingDirectory().empty())
		{
			os << L"<LocalDebuggerWorkingDirectory>" << resolveEnv(configuration->getDebugWorkingDirectory(), nullptr) << L"</LocalDebuggerWorkingDirectory>" << Endl;
			needed = true;
		}

		if (!configuration->getDebugArguments().empty())
		{
			os << L"<LocalDebuggerCommandArguments>" << resolveEnv(configuration->getDebugArguments(), nullptr) << L"</LocalDebuggerCommandArguments>" << Endl;
			needed = true;
		}

		os << DecreaseIndent;
		os << L"</PropertyGroup>" << Endl;
	}

	os << DecreaseIndent;
	os << L"</Project>" << Endl;

	os.close();

	if (needed)
	{
		if (!writeFileIfMismatch(projectFileName + L".user", buffer))
			return false;
	}

	return true;
}

bool SolutionBuilderMsvcVCXProj::collectFiles(
	const Project* project,
	const ProjectItem* item,
	const std::wstring& filterPath,
	std::vector< std::pair< std::wstring, Path > >& outFiles) const
{
	Ref< const Filter > filter = dynamic_type_cast< const Filter* >(item);
	if (filter)
	{
		std::wstring childFilterPath;

		if (!filterPath.empty())
			childFilterPath = filterPath + L"\\" + filter->getName();
		else
			childFilterPath = filter->getName();

		for (auto it : item->getItems())
			collectFiles(
				project,
				it,
				childFilterPath,
				outFiles);
	}

	Ref< const sb::File > file = dynamic_type_cast< const sb::File* >(item);
	if (file)
	{
		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);
		for (const auto& systemFile : systemFiles)
			outFiles.push_back(std::make_pair(filterPath, systemFile));
	}

	return true;
}

void SolutionBuilderMsvcVCXProj::findDefinitions(
	GeneratorContext& context,
	const Solution* solution,
	const Project* project,
	const RefArray< ProjectItem >& items) const
{
	const Path rootPath = FileSystem::getInstance().getAbsolutePath(context.get(L"PROJECT_PATH"));
	for (auto item : items)
	{
		if (auto file = dynamic_type_cast< const sb::File* >(item))
		{
			std::set< Path > systemFiles;
			file->getSystemFiles(project->getSourcePath(), systemFiles);
			for (const auto& systemFile : systemFiles)
			{
				if (compareIgnoreCase(systemFile.getExtension(), L"def") == 0)
				{
					Path relativePath;
					FileSystem::getInstance().getRelativePath(
						systemFile,
						rootPath,
						relativePath);
					context.set(L"MODULE_DEFINITION_FILE", relativePath.getPathName());
				}
			}
		}
		findDefinitions(context, solution, project, item->getItems());
	}
}

}

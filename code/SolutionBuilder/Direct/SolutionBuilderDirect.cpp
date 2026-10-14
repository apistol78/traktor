/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "SolutionBuilder/Direct/SolutionBuilderDirect.h"

#include "Core/Containers/SmallSet.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/File.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/MD5.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Core/System/ResolveEnv.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/JobQueue.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Timer/Timer.h"
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/File.h"
#include "SolutionBuilder/Filter.h"
#include "SolutionBuilder/HeaderScanner.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Utilities.h"
#include "Xml/XmlDeserializer.h"

#include <limits>
#include <map>
#include <set>

namespace traktor::sb
{
namespace
{

#if defined(_WIN32)
const wchar_t* c_platform = L"win64";
#elif defined(__APPLE__)
const wchar_t* c_platform = L"osx";
#elif defined(__RPI__)
const wchar_t* c_platform = L"rpi";
#else
const wchar_t* c_platform = L"linux";
#endif

const wchar_t* c_formatNames[] = { L"StaticLibrary", L"SharedLibrary", L"Executable", L"ExecutableConsole" };
const wchar_t* c_profileNames[] = { L"Debug", L"Release" };
const wchar_t* c_warningLevelNames[] = { L"NoWarnings", L"CriticalOnly", L"CompilerDefault", L"AllWarnings" };

// Each process occupies two of the thread pool's 256 workers reading its pipes.
const uint32_t c_maxJobs = 120;

Semaphore s_executeLock;

typedef SmallMap< std::wstring, std::wstring > values_t;

Ref< PropertyGroup > loadSettings(const Path& fileName)
{
	Ref< IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return nullptr;

	Ref< PropertyGroup > settings = xml::XmlDeserializer(file, fileName.getPathName()).readObject< PropertyGroup >();
	file->close();

	if (!settings)
		log::error << L"Unable to parse settings \"" << fileName.getPathName() << L"\"." << Endl;

	return settings;
}

const PropertyGroup* getGroup(const PropertyGroup* group, const std::wstring& name)
{
	return group ? dynamic_type_cast< const PropertyGroup* >(group->getProperty(name)) : nullptr;
}

std::wstring getString(const PropertyGroup* group, const std::wstring& name)
{
	return group ? group->getProperty< std::wstring >(name, L"") : L"";
}

/*! Replace all %(Key) with values; unknown keys are kept as is. */
std::wstring expand(const std::wstring& str, const values_t& values)
{
	std::wstring out = str;
	size_t offset = 0;
	for (;;)
	{
		const size_t s = out.find(L"%(", offset);
		if (s == out.npos)
			break;

		const size_t e = out.find(L')', s + 2);
		if (e == out.npos)
			break;

		const auto it = values.find(out.substr(s + 2, e - s - 2));
		if (it != values.end())
		{
			out = out.substr(0, s) + it->second + out.substr(e + 1);
			offset = s + it->second.length();
		}
		else
			offset = e + 1;
	}
	return out;
}

std::wstring quote(const std::wstring& str)
{
	return L"\"" + str + L"\"";
}

bool hasExtension(const PropertyGroup* group, const std::wstring& extension)
{
	for (const auto& e : StringSplit< std::wstring >(getString(group, L"Extensions"), L";"))
		if (compareIgnoreCase(e, extension) == 0)
			return true;
	return false;
}

/*! Find group, in a group of groups, which handles given file extension. */
const PropertyGroup* findByExtension(const PropertyGroup* groups, const std::wstring& extension)
{
	if (!groups)
		return nullptr;

	for (const auto& it : groups->getValues())
	{
		const PropertyGroup* group = dynamic_type_cast< const PropertyGroup* >(it.second);
		if (group && hasExtension(group, extension))
			return group;
	}

	return nullptr;
}

void collectFiles(const Project* project, const ProjectItem* item, std::set< Path >& outFiles)
{
	if (const sb::File* file = dynamic_type_cast< const sb::File* >(item))
		file->getSystemFiles(project->getSourcePath(), outFiles);
	else if (is_a< Filter >(item))
		for (auto child : item->getItems())
			collectFiles(project, child, outFiles);
}

template < typename ValueType >
void insertLast(AlignedVector< ValueType >& values, const ValueType& value)
{
	auto it = std::find(values.begin(), values.end(), value);
	if (it != values.end())
		values.erase(it);
	values.push_back(value);
}

void setVariable(std::map< std::wstring, std::wstring >& environment, const std::wstring& name, const std::wstring& value)
{
#if defined(_WIN32)
	for (auto it = environment.begin(); it != environment.end();)
		if (compareIgnoreCase(it->first, name) == 0)
			it = environment.erase(it);
		else
			++it;
#endif
	environment[name] = value;
}

std::wstring hashCommand(const std::wstring& str)
{
	MD5 md5;
	md5.begin();
	md5.feed(str);
	md5.end();
	return md5.format();
}

/*! Execute command and collect output, lines equal to ignoreLine are skipped. */
int32_t run(const std::wstring& command, const Path& workingDirectory, const Environment* environment, const std::wstring& ignoreLine, std::wstring& outOutput)
{
	// Serialize process creation so children do not inherit pipe handles of each other.
	Ref< IProcess > process;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(s_executeLock);
		process = OS::getInstance().execute(command, workingDirectory, environment, OS::EfRedirectStdIO | OS::EfMute);
	}
	if (!process)
	{
		outOutput = L"Unable to execute \"" + command + L"\"\n";
		return -1;
	}

	PipeReader stdOutReader(process->getPipeStream(IProcess::SpStdOut));
	PipeReader stdErrReader(process->getPipeStream(IProcess::SpStdErr));

	StringOutputStream ss;
	std::wstring line;
	for (;;)
	{
		Ref< IStream > pipe;
		const IProcess::WaitPipeResult result = process->waitPipeStream(100, pipe);
		if (result == IProcess::Ready && pipe != nullptr)
		{
			PipeReader& reader = (pipe == process->getPipeStream(IProcess::SpStdOut)) ? stdOutReader : stdErrReader;
			while (reader.readLine(line) == PipeReader::RtOk)
				if (line != ignoreLine)
					ss << line << L"\n";
		}
		else if (result == IProcess::Terminated)
			break;
	}

	outOutput = ss.str();
	return process->exitCode();
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionBuilderDirect", SolutionBuilderDirect, SolutionBuilder)

SolutionBuilderDirect::SolutionBuilderDirect()
:	m_abort(false)
{
}

SolutionBuilderDirect::~SolutionBuilderDirect()
{
	for (auto task : m_tasks)
		delete task;
	for (auto unit : m_unitOrder)
		delete unit;
}

bool SolutionBuilderDirect::create(const CommandLine& cmdLine)
{
	m_platform = c_platform;

	const Path settingsPath = FileSystem::getInstance().getAbsolutePath(Path(L"$(TRAKTOR_HOME)/resources/build/configurations/sb-direct.xml"));
	const Path platformSettingsPath = settingsPath.getPathNameNoExtension() + L"." + m_platform + L"." + settingsPath.getExtension();

	m_settings = loadSettings(settingsPath);
	if (!m_settings)
	{
		log::error << L"Unable to read settings \"" << settingsPath.getPathName() << L"\"." << Endl;
		return false;
	}

	if (FileSystem::getInstance().exist(platformSettingsPath))
	{
		Ref< PropertyGroup > platformSettings = loadSettings(platformSettingsPath);
		if (!platformSettings)
			return false;

		m_settings = m_settings->merge(platformSettings, PropertyGroup::MmJoin);
	}

	// Extra environment variables, may reference existing variables.
	std::map< std::wstring, std::wstring > environment = OS::getInstance().getEnvironment()->get();
	if (auto variables = getGroup(m_settings, L"Direct.Environment"))
		for (const auto& it : variables->getValues())
			setVariable(environment, it.first, resolveEnv(PropertyString::get(it.second), nullptr));
	m_environment = new Environment();
	m_environment->insert(environment);

	if (cmdLine.hasOption(L"rebuild"))
	{
		m_rebuildProject = cmdLine.getOption(L"rebuild").getString();
		m_rebuildAll = m_rebuildProject.empty();
	}
	if (cmdLine.hasOption(L"target"))
		m_target = cmdLine.getOption(L"target").getString();
	if (cmdLine.hasOption(L"launch"))
		m_launch = cmdLine.getOption(L"launch").getString();

	if (!m_launch.empty() && m_target.empty())
	{
		log::error << L"Launch requires a target." << Endl;
		return false;
	}

	for (uint32_t i = 1; i < cmdLine.getCount(); ++i)
		m_projects.push_back(cmdLine.getString(i));

	m_dependentOnly = cmdLine.hasOption(L"dependent-only");
	m_verbose = cmdLine.hasOption('v', L"verbose");
	m_headerScanner = new HeaderScanner();
	return true;
}

bool SolutionBuilderDirect::generate(const Solution* solution, const Path& solutionPathName)
{
	Timer timer;

	// Create units of all enabled projects and their dependencies.
	const Path solutionPathNameAbs = FileSystem::getInstance().getAbsolutePath(solutionPathName);
	for (auto project : solution->getProjects())
	{
		if (!project->getEnable())
			continue;
		if (!m_projects.empty() && std::find(m_projects.begin(), m_projects.end(), project->getName()) == m_projects.end())
			continue;

		for (auto configuration : project->getConfigurations())
		{
			if (m_target.empty() || compareIgnoreCase(configuration->getName(), m_target) == 0)
			{
				if (!addUnit(solution, solutionPathNameAbs, project, configuration->getName()))
					return false;
			}
		}
	}

	if (m_unitOrder.empty())
	{
		log::warning << L"Nothing to build." << Endl;
		return true;
	}

	// Create tasks; units are ordered so dependencies are planned first.
	log::info << L"Scanning " << (int32_t)m_unitOrder.size() << L" project configuration(s)..." << Endl;
	for (auto unit : m_unitOrder)
		if (!planUnit(unit))
			return false;

	if (!execute())
	{
		log::error << L"Build failed after " << int32_t(timer.getElapsedTime() + 0.5) << L" second(s)." << Endl;
		return false;
	}

	log::info << L"Build succeeded in " << int32_t(timer.getElapsedTime() + 0.5) << L" second(s)." << Endl;

	if (!m_launch.empty())
		return launch();

	return true;
}

void SolutionBuilderDirect::showOptions() const
{
	log::info << L"\t--rebuild              Force rebuild everything" << Endl;
	log::info << L"\t--rebuild=[project]    Force rebuild project" << Endl;
	log::info << L"\t--target=[target]      Build only target (DebugStatic, ReleaseStatic, DebugShared, ReleaseShared)" << Endl;
	log::info << L"\t--launch=[project]     Launch project after build, require target" << Endl;
	log::info << L"\t--dependent-only       Build only dependent projects of external solutions" << Endl;
}

SolutionBuilderDirect::Unit* SolutionBuilderDirect::addUnit(const Solution* solution, const Path& solutionPathName, const Project* project, const std::wstring& target)
{
	const auto key = std::make_pair(project, target);

	auto it = m_units.find(key);
	if (it != m_units.end())
		return it->second;

	const Configuration* configuration = project->getConfiguration(target);
	if (!configuration)
	{
		log::error << L"No configuration \"" << target << L"\" in project \"" << project->getName() << L"\"." << Endl;
		return nullptr;
	}

	Unit* unit = new Unit();
	unit->solution = solution;
	unit->project = project;
	unit->configuration = configuration;
	unit->enabled = project->getEnable();
	unit->target = target;
	unit->rebuild = m_rebuildAll || project->getName() == m_rebuildProject;
	m_units[key] = unit;

	const Path rootPath = FileSystem::getInstance().getAbsolutePath(Path(solutionPathName.getPathOnly()), Path(solution->getRootPath()));
	unit->outDir = rootPath + Path(target);
	unit->intDir = rootPath + Path(project->getName()) + Path(target);

	const PropertyGroup* products = getGroup(m_settings, L"Direct.Products");
	const std::wstring productName = expand(getString(products, c_formatNames[configuration->getTargetFormat()]), { { L"Name", project->getName() } });
	if (productName.empty())
	{
		log::error << L"No product name for " << c_formatNames[configuration->getTargetFormat()] << L"." << Endl;
		return nullptr;
	}
	unit->productPath = unit->outDir + Path(productName);

	// Include and library paths, also inherited from dependencies.
	SmallSet< const Project* > visited;
	SmallMap< std::wstring, SmallSet< std::wstring > > includePaths;
	SmallMap< std::wstring, SmallSet< std::wstring > > libraryPaths;
	flattenIncludePaths(solutionPathName, project, visited, includePaths, libraryPaths);
	unit->includePaths = AlignedVector< std::wstring >(includePaths[target].begin(), includePaths[target].end());
	unit->libraryPaths = AlignedVector< std::wstring >(libraryPaths[target].begin(), libraryPaths[target].end());

	for (auto dependency : project->getDependencies())
	{
		Unit* dependencyUnit = nullptr;
		if (auto projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency))
			dependencyUnit = addUnit(solution, solutionPathName, projectDependency->getProject(), target);
		else if (auto externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency))
		{
			const Path externalSolutionPathName = FileSystem::getInstance().getAbsolutePath(Path(solutionPathName.getPathOnly()), Path(externalDependency->getSolutionFileName()));
			dependencyUnit = addUnit(externalDependency->getSolution(), externalSolutionPathName, externalDependency->getProject(), target);
			if (dependencyUnit && !m_dependentOnly && !addSolution(externalDependency->getSolution(), externalSolutionPathName, target))
				return nullptr;
		}
		if (!dependencyUnit)
			return nullptr;

		unit->dependencies.push_back({ dependencyUnit, dependency->getLink() != Dependency::LnkNo });
	}

	m_unitOrder.push_back(unit);
	return unit;
}

bool SolutionBuilderDirect::addSolution(const Solution* solution, const Path& solutionPathName, const std::wstring& target)
{
	if (!m_solutions.insert(solution))
		return true;

	for (auto project : solution->getProjects())
	{
		if (project->getEnable() && project->getConfiguration(target) != nullptr)
		{
			if (!addUnit(solution, solutionPathName, project, target))
				return false;
		}
	}

	return true;
}

bool SolutionBuilderDirect::planUnit(Unit* unit)
{
	const Project* project = unit->project;
	const Configuration* configuration = unit->configuration;
	const Configuration::TargetFormat targetFormat = configuration->getTargetFormat();

	// Disabled projects are prebuilt, only used for their include paths and libraries.
	if (!unit->enabled)
		return true;

	loadHashes(unit);

	const values_t common = {
		{ L"Platform", m_platform },
		{ L"Name", project->getName() },
		{ L"Configuration", unit->target },
		{ L"OutDir", unit->outDir.getPathNameOS() },
		{ L"IntDir", unit->intDir.getPathNameOS() }
	};

	std::set< Path > files;
	for (auto item : project->getItems())
		collectFiles(project, item, files);

	AlignedVector< Task* > toolTasks;

	// Tools generating files, for example embedded resources.
	const PropertyGroup* tools = getGroup(m_settings, L"Direct.Tools");
	for (const auto& file : files)
	{
		const PropertyGroup* tool = findByExtension(tools, file.getExtension());
		if (!tool)
			continue;

		values_t values = common;
		values[L"Input"] = file.getPathNameOS();
		values[L"FileName"] = file.getFileNameNoExtension();

		const Path output = Path(expand(getString(tool, L"Output"), values)).normalized();
		values[L"Output"] = output.getPathNameOS();

		Task* task = new Task();
		task->unit = unit;
		task->message = L"Generating " + output.getFileName();
		task->command = resolveEnv(expand(getString(tool, L"Command"), values), m_environment);
		task->hash = hashCommand(task->command);
		task->output = output;

		const uint64_t outputTimeStamp = getTimeStamp(output.getPathNameOS());
		task->needed =
			unit->rebuild ||
			outputTimeStamp == 0 ||
			getTimeStamp(file.getPathNameOS()) > outputTimeStamp ||
			unit->hashes[output.getPathName()] != task->hash;

		// Generated files are newer than everything which include them.
		if (task->needed)
			m_timeStamps[output.getPathNameOS()] = std::numeric_limits< uint64_t >::max();

		toolTasks.push_back(task);
		m_tasks.push_back(task);
	}

	// Compile sources.
	const PropertyGroup* compilers = getGroup(m_settings, L"Direct.Compilers");
	const Path sourcePath = FileSystem::getInstance().getAbsolutePath(Path(project->getSourcePath())).normalized();

	m_headerScanner->removeAllIncludePaths();
	m_headerScanner->addIncludePath(unit->intDir.getPathNameOS());
	for (const auto& includePath : unit->includePaths)
		if (!startsWith(includePath, L"`"))
			m_headerScanner->addIncludePath(includePath);

	AlignedVector< Task* > compileTasks;
	AlignedVector< std::wstring > objects;
	Path definitionFile;

	for (const auto& file : files)
	{
		if (compareIgnoreCase(file.getExtension(), L"def") == 0)
		{
			definitionFile = file;
			continue;
		}

		const PropertyGroup* compiler = findByExtension(compilers, file.getExtension());
		if (!compiler)
			continue;

		// Mirror source folder structure; parent references are renamed to keep objects inside intermediate folder.
		Path relativePath;
		if (!FileSystem::getInstance().getRelativePath(file, sourcePath, relativePath))
			relativePath = Path(file.getFileName());
		const std::wstring objectName = replaceAll(relativePath.getPathNameNoExtension(), L"..", L"__") + getString(compiler, L"ObjectExtension");
		const Path output = unit->intDir + Path(objectName);

		StringOutputStream includePaths;
		includePaths << expand(getString(compiler, L"IncludePath"), { { L"Value", unit->intDir.getPathNameOS() } });
		for (const auto& includePath : unit->includePaths)
			if (!startsWith(includePath, L"`"))
				includePaths << L" " << expand(getString(compiler, L"IncludePath"), { { L"Value", includePath } });
			else
				includePaths << L" " << evaluate(includePath);

		StringOutputStream definitions;
		for (const auto& definition : configuration->getDefinitions())
		{
			if (!definitions.empty())
				definitions << L" ";
			if (startsWith(definition, L"`"))
				definitions << evaluate(definition);
			else if (startsWith(definition, L"'"))
				definitions << definition;
			else
				definitions << expand(getString(compiler, L"Definition"), { { L"Value", definition } });
		}

		values_t values = common;
		values[L"Input"] = file.getPathNameOS();
		values[L"Output"] = output.getPathNameOS();
		values[L"Options"] = getOptions(compiler, unit);
		values[L"IncludePaths"] = includePaths.str();
		values[L"Definitions"] = definitions.str();
		values[L"WarningLevel"] = getString(getGroup(compiler, L"WarningLevels"), c_warningLevelNames[configuration->getWarningLevel()]);
		values[L"AdditionalOptions"] = configuration->getAdditionalCompilerOptions();

		Task* task = new Task();
		task->unit = unit;
		task->message = L"Compiling " + relativePath.getPathName();
		task->command = resolveEnv(getString(compiler, L"Executable") + L" " + expand(getString(compiler, L"Arguments"), values), m_environment);
		task->hash = hashCommand(task->command);
		task->output = output;
		task->echo = file.getFileName();

		const uint64_t outputTimeStamp = getTimeStamp(output.getPathNameOS());
		task->needed =
			unit->rebuild ||
			outputTimeStamp == 0 ||
			getTimeStamp(file.getPathNameOS()) > outputTimeStamp ||
			unit->hashes[output.getPathName()] != task->hash;

		if (!task->needed)
			task->needed = getIncludedTimeStamp(file.getPathNameOS()) > outputTimeStamp;

		for (auto toolTask : toolTasks)
		{
			toolTask->dependents.push_back(task);
			task->pending++;
		}

		objects.push_back(output.getPathNameOS());
		compileTasks.push_back(task);
		m_tasks.push_back(task);
	}

	// Link or archive product, then aggregate.
	Task* task = new Task();
	task->unit = unit;
	task->output = unit->productPath;
	task->aggregate = true;
	unit->linkTask = task;
	m_tasks.push_back(task);

	for (auto compileTask : compileTasks)
	{
		compileTask->dependents.push_back(task);
		task->pending++;
	}
	for (const auto& dependency : unit->dependencies)
	{
		if (dependency.first->linkTask)
		{
			dependency.first->linkTask->dependents.push_back(task);
			task->pending++;
		}
	}

	if (objects.empty())
		return true;

	StringOutputStream inputs;
	for (const auto& object : objects)
		inputs << quote(object) << L"\n";

	values_t values = common;
	values[L"Output"] = unit->productPath.getPathNameOS();
	values[L"Inputs"] = inputs.str();

	const PropertyGroup* tool = nullptr;
	if (targetFormat == Configuration::TfStaticLibrary)
	{
		tool = getGroup(m_settings, L"Direct.Archiver");
		task->message = L"Archiving " + unit->productPath.getFileName();
		task->archive = true;
	}
	else
	{
		tool = getGroup(m_settings, L"Direct.Linker");
		task->message = L"Linking " + unit->productPath.getFileName();

		collectLibraries(unit);

		StringOutputStream libraryPaths;
		libraryPaths << expand(getString(tool, L"LibraryPath"), { { L"Value", unit->outDir.getPathNameOS() } });
		for (const auto& libraryPath : unit->collectedLibraryPaths)
			if (!startsWith(libraryPath, L"`"))
				libraryPaths << L" " << expand(getString(tool, L"LibraryPath"), { { L"Value", libraryPath } });
			else
				libraryPaths << L" " << evaluate(libraryPath);

		StringOutputStream libraries;
		for (auto library : unit->collectedLibraries)
			libraries << expand(getString(tool, L"Library"), { { L"Value", library->project->getName() } }) << L" ";
		for (const auto& library : unit->collectedExternalLibraries)
			if (!startsWith(library, L"`"))
				libraries << expand(getString(tool, L"ExternalLibrary"), { { L"Value", library } }) << L" ";
			else
				libraries << evaluate(library) << L" ";

		values[L"LibraryPaths"] = libraryPaths.str();
		values[L"Libraries"] = libraries.str();
		values[L"DefinitionFile"] = !definitionFile.empty() ? expand(getString(tool, L"DefinitionFile"), { { L"Value", definitionFile.getPathNameOS() } }) : L"";
		values[L"AdditionalOptions"] = configuration->getAdditionalLinkerOptions();
	}

	if (!tool)
	{
		log::error << L"No linker or archiver configured." << Endl;
		return false;
	}

	values[L"Options"] = getOptions(tool, unit);

	task->responseFile = (unit->intDir + Path(L"sb-direct.rsp")).getPathNameOS();
	task->responseContent = resolveEnv(expand(getString(tool, L"Arguments"), values), m_environment);
	task->command = resolveEnv(getString(tool, L"Executable"), m_environment) + L" @" + quote(task->responseFile);
	task->hash = hashCommand(task->command + task->responseContent);

	const uint64_t productTimeStamp = getTimeStamp(unit->productPath.getPathNameOS());
	task->needed =
		unit->rebuild ||
		productTimeStamp == 0 ||
		unit->hashes[unit->productPath.getPathName()] != task->hash;

	for (auto compileTask : compileTasks)
		task->needed |= compileTask->needed || getTimeStamp(compileTask->output.getPathNameOS()) > productTimeStamp;

	// Archives do not contain dependencies so only linked products depend on them.
	if (targetFormat != Configuration::TfStaticLibrary)
	{
		for (const auto& dependency : unit->dependencies)
			if (dependency.first->linkTask && dependency.second)
				task->needed |= dependency.first->linkTask->needed || getTimeStamp(dependency.first->productPath.getPathNameOS()) > productTimeStamp;
	}

	return true;
}

void SolutionBuilderDirect::collectLibraries(Unit* unit)
{
	if (unit->collected)
		return;

	unit->collected = true;

	for (const auto& library : unit->configuration->getLibraries())
		insertLast(unit->collectedExternalLibraries, library);
	for (const auto& libraryPath : unit->libraryPaths)
		insertLast(unit->collectedLibraryPaths, libraryPath);

	for (const auto& it : unit->dependencies)
	{
		Unit* dependency = it.first;
		if (dependency->enabled && it.second)
		{
			insertLast(unit->collectedLibraries, dependency);
			insertLast(unit->collectedLibraryPaths, dependency->outDir.getPathNameOS());
		}

		const std::wstring& consumerLibraryPath = dependency->configuration->getConsumerLibraryPath();
		if (!consumerLibraryPath.empty())
		{
			const Path libraryPath = FileSystem::getInstance().getAbsolutePath(Path(dependency->solution->getAggregateOutputPath()) + Path(consumerLibraryPath));
			insertLast(unit->collectedLibraryPaths, libraryPath.getPathNameOS());
		}

		// Static libraries need their dependencies linked by consumer.
		if (dependency->configuration->getTargetFormat() == Configuration::TfStaticLibrary)
		{
			collectLibraries(dependency);
			for (auto library : dependency->collectedLibraries)
				insertLast(unit->collectedLibraries, library);
			for (const auto& library : dependency->collectedExternalLibraries)
				insertLast(unit->collectedExternalLibraries, library);
			for (const auto& libraryPath : dependency->collectedLibraryPaths)
				insertLast(unit->collectedLibraryPaths, libraryPath);
		}
	}
}

bool SolutionBuilderDirect::execute()
{
	int32_t total = 0;
	for (auto task : m_tasks)
		if (task->needed)
			++total;

	const uint32_t jobs = std::min(OS::getInstance().getCPUCoreCount(), c_maxJobs);
	log::info << L"Building " << total << L" task(s) using " << jobs << L" job(s)..." << Endl;

	JobQueue queue;
	if (!queue.create(jobs, Thread::Normal))
		return false;

	Semaphore lock;
	Event finishedEvent;
	AlignedVector< Task* > finished;
	AlignedVector< Task* > ready;
	int32_t outstanding = 0;
	int32_t completed = 0;
	bool failed = false;

	for (auto task : m_tasks)
		if (task->pending == 0)
			ready.push_back(task);

	for (;;)
	{
		for (auto task : ready)
		{
			++outstanding;
			queue.add([&, task]() {
				executeTask(task);
				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(lock);
					finished.push_back(task);
				}
				finishedEvent.pulse();
			});
		}
		ready.clear();

		if (outstanding <= 0)
			break;

		finishedEvent.wait(100);

		AlignedVector< Task* > done;
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(lock);
			done.swap(finished);
		}

		for (auto task : done)
		{
			--outstanding;

			Unit* unit = task->unit;
			const std::wstring output = task->output.getPathName();

			if (task->needed && (task->succeeded || !task->log.empty()))
				log::info << L"[" << ++completed << L"/" << total << L"] " << unit->project->getName() << L" (" << unit->target << L"): " << task->message << Endl;

			if (!task->log.empty())
			{
				if (task->succeeded)
					log::info << task->log;
				else
					log::error << task->log;
			}

			if (task->succeeded)
			{
				if (task->needed && !task->hash.empty())
				{
					unit->hashes[output] = task->hash;
					unit->hashesModified = true;
				}
				for (auto dependent : task->dependents)
					if (--dependent->pending == 0)
						ready.push_back(dependent);
			}
			else
			{
				if (unit->hashes.remove(output))
					unit->hashesModified = true;
				failed = true;
				m_abort = true;
			}

			// Save state as soon as unit is finished so an interrupted build keeps it.
			if (task == unit->linkTask)
				saveHashes(unit);
		}
	}

	queue.destroy();

	for (auto unit : m_unitOrder)
		saveHashes(unit);

	return !failed;
}

void SolutionBuilderDirect::executeTask(Task* task)
{
	if (m_abort)
		return;

	if (task->needed)
	{
		if (m_verbose)
			task->log = task->command + L"\n" + task->responseContent + L"\n";

		FileSystem::getInstance().makeAllDirectories(task->output.getPathOnly());

		if (task->archive)
			FileSystem::getInstance().remove(task->output);

		if (!task->responseFile.empty())
		{
			FileSystem::getInstance().makeAllDirectories(task->unit->intDir);

			Ref< IStream > file = FileSystem::getInstance().open(task->responseFile, traktor::File::FmWrite);
			if (!file)
			{
				task->log += L"Unable to create response file \"" + task->responseFile + L"\".\n";
				return;
			}
			FileOutputStream(file, new Utf8Encoding()) << task->responseContent;
			file->close();
		}

		std::wstring output;
		const int32_t exitCode = run(task->command, task->unit->intDir, m_environment, task->echo, output);
		task->log += output;

		if (exitCode != 0)
		{
			task->log += task->message + L" failed, exit code " + toString(exitCode) + L".\n";
			return;
		}
	}

	if (task->aggregate && !aggregate(task))
		return;

	task->succeeded = true;
}

bool SolutionBuilderDirect::aggregate(Task* task)
{
	const Unit* unit = task->unit;
	const Path aggregateOutputPath = FileSystem::getInstance().getAbsolutePath(Path(unit->solution->getAggregateOutputPath()));

	for (auto aggregationItem : unit->configuration->getAggregationItems())
	{
		const Path sourceFile = FileSystem::getInstance().getAbsolutePath(unit->outDir, Path(aggregationItem->getSourceFile()));
		const Path targetPath = aggregateOutputPath + Path(aggregationItem->getTargetPath());

		RefArray< traktor::File > sourceFiles = FileSystem::getInstance().find(sourceFile);
		if (sourceFiles.empty())
		{
			if (m_verbose)
				task->log += L"No file \"" + sourceFile.getPathName() + L"\" to aggregate.\n";
			continue;
		}

		FileSystem::getInstance().makeAllDirectories(targetPath);

		for (auto file : sourceFiles)
		{
			if (file->isDirectory())
				continue;

			const Path targetFile = targetPath + Path(file->getPath().getFileName());

			Ref< traktor::File > existing = FileSystem::getInstance().get(targetFile);
			if (existing && existing->getLastWriteTime().getSecondsSinceEpoch() >= file->getLastWriteTime().getSecondsSinceEpoch())
				continue;

			if (!FileSystem::getInstance().copy(targetFile, file->getPath(), true))
			{
				task->log += L"Unable to copy \"" + file->getPath().getPathName() + L"\" to \"" + targetFile.getPathName() + L"\".\n";
				return false;
			}

			FileSystem::getInstance().modify(targetFile, file->getFlags());
			task->log += L"Copied " + file->getPath().getFileName() + L" to " + targetPath.getPathName() + L"\n";
		}
	}

	return true;
}

bool SolutionBuilderDirect::launch()
{
	const Unit* unit = nullptr;
	for (auto it : m_unitOrder)
	{
		if (it->project->getName() == m_launch && compareIgnoreCase(it->target, m_target) == 0)
		{
			unit = it;
			break;
		}
	}
	if (!unit)
	{
		log::error << L"Unable to launch \"" << m_launch << L"\"; project not built." << Endl;
		return false;
	}

	const Configuration* configuration = unit->configuration;

	std::map< std::wstring, std::wstring > variables = m_environment->get();
	setVariable(variables, L"TargetPath", unit->productPath.getPathNameOS());
	setVariable(variables, L"TargetDir", unit->outDir.getPathNameOS());
	setVariable(variables, L"AGGREGATE_OUTPUT_PATH", FileSystem::getInstance().getAbsolutePath(Path(unit->solution->getAggregateOutputPath())).getPathNameOS());

	Ref< Environment > environment = new Environment();
	environment->insert(variables);

	// Debug environment has one "NAME=VALUE" per line.
	for (const auto& line : StringSplit< std::wstring >(configuration->getDebugEnvironment(), L"\r\n"))
	{
		const size_t p = line.find(L'=');
		if (p != line.npos)
			setVariable(variables, trim(line.substr(0, p)), resolveEnv(line.substr(p + 1), environment));
	}

	environment = new Environment();
	environment->insert(variables);

	std::wstring executable = resolveEnv(configuration->getDebugExecutable(), environment);
	if (executable.empty())
		executable = unit->productPath.getPathNameOS();

	Path workingDirectory = Path(resolveEnv(configuration->getDebugWorkingDirectory(), environment));
	if (workingDirectory.empty())
		workingDirectory = unit->outDir;

	const std::wstring arguments = resolveEnv(configuration->getDebugArguments(), environment);

	log::info << L"Launching \"" << executable << L"\"..." << Endl;

	if (!OS::getInstance().execute(quote(executable) + L" " + arguments, workingDirectory, environment, OS::EfNone))
	{
		log::error << L"Unable to launch \"" << executable << L"\"." << Endl;
		return false;
	}

	return true;
}

std::wstring SolutionBuilderDirect::evaluate(const std::wstring& value)
{
	auto it = m_evaluated.find(value);
	if (it != m_evaluated.end())
		return it->second;

	const std::wstring command = trim(replaceAll(value, L"`", L""));

	std::wstring output;
	if (run(command, FileSystem::getInstance().getCurrentVolumeAndDirectory(), m_environment, L"", output) != 0)
		log::warning << L"Command \"" << command << L"\" failed; " << output << Endl;

	output = trim(replaceAll(replaceAll(output, L'\n', L' '), L'\r', L' '));
	m_evaluated[value] = output;
	return output;
}

uint64_t SolutionBuilderDirect::getTimeStamp(const std::wstring& fileName)
{
	auto it = m_timeStamps.find(fileName);
	if (it != m_timeStamps.end())
		return it->second;

	Ref< traktor::File > file = FileSystem::getInstance().get(Path(fileName));
	const uint64_t timeStamp = file ? file->getLastWriteTime().getSecondsSinceEpoch() : 0;
	m_timeStamps[fileName] = timeStamp;
	return timeStamp;
}

int32_t SolutionBuilderDirect::getHeader(const std::wstring& fileName)
{
	auto it = m_headerIds.find(fileName);
	if (it != m_headerIds.end())
		return it->second;

	const int32_t id = (int32_t)m_headers.size();
	m_headers.push_back({ fileName, getTimeStamp(fileName) });
	m_headerIds[fileName] = id;
	return id;
}

uint64_t SolutionBuilderDirect::getIncludedTimeStamp(const std::wstring& fileName)
{
	AlignedVector< int32_t > stack;
	stack.push_back(getHeader(Path(fileName).normalized().getPathNameOS()));

	uint64_t newest = 0;
	++m_visit;

	while (!stack.empty())
	{
		const int32_t id = stack.back();
		stack.pop_back();

		if (!m_headers[id].scanned)
		{
			AlignedVector< int32_t > includes;
			for (const auto& include : m_headerScanner->getIncludes(m_headers[id].fileName))
				includes.push_back(getHeader(include));

			m_headers[id].includes = includes;
			m_headers[id].scanned = true;
		}

		for (int32_t include : m_headers[id].includes)
		{
			Header& header = m_headers[include];
			if (header.visited == m_visit)
				continue;

			header.visited = m_visit;
			newest = std::max(newest, header.timeStamp);
			stack.push_back(include);
		}
	}

	return newest;
}

std::wstring SolutionBuilderDirect::getOptions(const PropertyGroup* tool, const Unit* unit) const
{
	const std::wstring target = std::wstring(c_profileNames[unit->configuration->getTargetProfile()]) + c_formatNames[unit->configuration->getTargetFormat()];
	return getString(getGroup(tool, L"Options"), target);
}

void SolutionBuilderDirect::loadHashes(Unit* unit) const
{
	Ref< IStream > file = FileSystem::getInstance().open(unit->intDir + Path(L"sb-direct.hashes"), traktor::File::FmRead);
	if (!file)
		return;

	BufferedStream bufferedFile(file);
	StringReader reader(&bufferedFile, new Utf8Encoding());

	std::wstring line;
	while (reader.readLine(line) >= 0)
	{
		const size_t p = line.find(L'\t');
		if (p != line.npos)
			unit->hashes[line.substr(p + 1)] = line.substr(0, p);
	}

	file->close();
}

void SolutionBuilderDirect::saveHashes(Unit* unit) const
{
	if (!unit->hashesModified)
		return;

	unit->hashesModified = false;

	FileSystem::getInstance().makeAllDirectories(unit->intDir);

	Ref< IStream > file = FileSystem::getInstance().open(unit->intDir + Path(L"sb-direct.hashes"), traktor::File::FmWrite);
	if (!file)
	{
		log::warning << L"Unable to save build state of \"" << unit->project->getName() << L"\"." << Endl;
		return;
	}

	FileOutputStream os(file, new Utf8Encoding());
	for (const auto& it : unit->hashes)
		if (!it.second.empty())
			os << it.second << L"\t" << it.first << Endl;
	os.close();
}

}

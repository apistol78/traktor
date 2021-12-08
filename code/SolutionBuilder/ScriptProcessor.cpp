#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/BoxedClassFactory.h"
#include "Core/Class/CoreClassFactory1.h"
#include "Core/Class/CoreClassFactory2.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptProfiler.h"
#include "Script/Lua/ScriptCompilerLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/HeaderScanner.h"
#include "SolutionBuilder/File.h"
#include "SolutionBuilder/Filter.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ProjectItem.h"
#include "SolutionBuilder/Solution.h"

#define T_PROFILER_ENABLE 0

namespace traktor
{
	namespace sb
	{
		namespace
		{

#if T_PROFILER_ENABLE
class ScriptProfilerListener : public script::IScriptProfiler::IListener
{
public:
	std::map< std::wstring, double > m_durations;

	virtual void callEnter(const Guid& scriptId, const std::wstring& function) override final
	{
	}

	virtual void callLeave(const Guid& scriptId, const std::wstring& function) override final
	{
	}

	virtual void callMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration) override final
	{
		m_durations[function] += exclusiveDuration;
	}
};
#endif

class Output : public Object
{
	T_RTTI_CLASS;

public:
	explicit Output(const std::vector< std::wstring >& sections)
	:	m_sections(sections)
	,	m_ss(1 * 1024 * 1024)
	{
	}

	void print(const std::wstring& str)
	{
		m_ss << str;
	}

	void printLn(const std::wstring& str)
	{
		m_ss << str << Endl;
	}

	void printSection(int32_t id)
	{
		T_FATAL_ASSERT(id >= 0 && id < int32_t(m_sections.size()));
		m_ss << m_sections[id];
	}

	std::wstring getProduct()
	{
		return m_ss.str();
	}

private:
	const std::vector< std::wstring >& m_sections;
	StringOutputStream m_ss;
};

T_IMPLEMENT_RTTI_CLASS(L"Output", Output, Object)

std::wstring Configuration_getTargetFormat(Configuration* configuration)
{
	switch (configuration->getTargetFormat())
	{
	case Configuration::TfStaticLibrary:
		return L"StaticLibrary";
	case Configuration::TfSharedLibrary:
		return L"SharedLibrary";
	case Configuration::TfExecutable:
		return L"Executable";
	case Configuration::TfExecutableConsole:
		return L"ExecutableConsole";
	default:
		return L"";
	}
}

std::wstring Configuration_getTargetProfile(Configuration* configuration)
{
	switch (configuration->getTargetProfile())
	{
	case Configuration::TpDebug:
		return L"Debug";
	case Configuration::TpRelease:
		return L"Release";
	default:
		return L"";
	}
}

std::wstring Dependency_getLink(Dependency* dependency)
{
	switch (dependency->getLink())
	{
	case Dependency::LnkNo:
		return L"No";
	case Dependency::LnkYes:
		return L"Yes";
	case Dependency::LnkForce:
		return L"Force";
	default:
		return L"";
	}
}

RefArray< Path > File_getSystemFiles(sb::File* file, const std::wstring& sourcePath)
{
	std::set< Path > systemFiles;
	file->getSystemFiles(sourcePath, systemFiles);

	RefArray< Path > systemFilesOut;
	for (auto systemFile : systemFiles)
		systemFilesOut.push_back(new Path(systemFile));

	return systemFilesOut;
}

Any HeaderScanner_get(HeaderScanner* self, const std::wstring& fileName, const std::wstring& projectPath)
{
	SmallSet< std::wstring > headerFiles;
	self->get(fileName, projectPath, headerFiles);
	return Any::fromObject(new BoxedAlignedVector(
		AlignedVector< std::wstring >(headerFiles.begin(), headerFiles.end())
	));
}

			}

T_IMPLEMENT_RTTI_CLASS(L"ScriptProcessor", ScriptProcessor, Object)

bool ScriptProcessor::create()
{
	// Create script manager and register our classes.
	m_scriptCompiler = new script::ScriptCompilerLua();
	m_scriptManager = new script::ScriptManagerLua();

	BoxedClassFactory().createClasses(m_scriptManager);
	CoreClassFactory1().createClasses(m_scriptManager);
	CoreClassFactory2().createClasses(m_scriptManager);

	auto classOutput = new AutoRuntimeClass< Output >();
	classOutput->addMethod("print", &Output::print);
	classOutput->addMethod("printLn", &Output::printLn);
	classOutput->addMethod("printSection", &Output::printSection);
	m_scriptManager->registerClass(classOutput);

	auto classSolution = new AutoRuntimeClass< Solution >();
	classSolution->addProperty("name", &Solution::getName);
	classSolution->addProperty("rootPath", &Solution::getRootPath);
	classSolution->addProperty("aggregateOutputPath", &Solution::getAggregateOutputPath);
	classSolution->addProperty("projects", &Solution::getProjects);
	classSolution->addMethod("getName", &Solution::getName);
	classSolution->addMethod("getRootPath", &Solution::getRootPath);
	classSolution->addMethod("getAggregateOutputPath", &Solution::getAggregateOutputPath);
	classSolution->addMethod("getProjects", &Solution::getProjects);
	m_scriptManager->registerClass(classSolution);

	auto classProject = new AutoRuntimeClass< Project >();
	classProject->addProperty("enable", &Project::getEnable);
	classProject->addProperty("name", &Project::getName);
	classProject->addProperty("sourcePath", &Project::getSourcePath);
	classProject->addProperty("configurations", &Project::getConfigurations);
	classProject->addProperty("items", &Project::getItems);
	classProject->addProperty("dependencies", &Project::getDependencies);
	classProject->addMethod("getEnable", &Project::getEnable);
	classProject->addMethod("getName", &Project::getName);
	classProject->addMethod("getSourcePath", &Project::getSourcePath);
	classProject->addMethod("getConfigurations", &Project::getConfigurations);
	classProject->addMethod("getConfiguration", &Project::getConfiguration);	// keep
	classProject->addMethod("getItems", &Project::getItems);
	classProject->addMethod("getDependencies", &Project::getDependencies);
	m_scriptManager->registerClass(classProject);

	auto classConfiguration = new AutoRuntimeClass< Configuration >();
	classConfiguration->addProperty("name", &Configuration::getName);
	classConfiguration->addProperty("targetFormat", &Configuration_getTargetFormat);
	classConfiguration->addProperty("targetProfile", &Configuration_getTargetProfile);
	classConfiguration->addProperty("precompiledHeader", &Configuration::getPrecompiledHeader);
	classConfiguration->addProperty("includePaths", &Configuration::getIncludePaths);
	classConfiguration->addProperty("definitions", &Configuration::getDefinitions);
	classConfiguration->addProperty("libraryPaths", &Configuration::getLibraryPaths);
	classConfiguration->addProperty("libraries", &Configuration::getLibraries);
	classConfiguration->addProperty("additionalCompilerOptions", &Configuration::getAdditionalCompilerOptions);
	classConfiguration->addProperty("additionalLinkerOptions", &Configuration::getAdditionalLinkerOptions);
	classConfiguration->addProperty("debugExecutable", &Configuration::getDebugExecutable);
	classConfiguration->addProperty("debugArguments", &Configuration::getDebugArguments);
	classConfiguration->addProperty("debugEnvironment", &Configuration::getDebugEnvironment);
	classConfiguration->addProperty("debugWorkingDirectory", &Configuration::getDebugWorkingDirectory);
	classConfiguration->addProperty("aggregationItems", &Configuration::getAggregationItems);
	classConfiguration->addProperty("consumerLibraryPath", &Configuration::getConsumerLibraryPath);
	classConfiguration->addMethod("getName", &Configuration::getName);
	classConfiguration->addMethod("getTargetFormat", &Configuration_getTargetFormat);
	classConfiguration->addMethod("getTargetProfile", &Configuration_getTargetProfile);
	classConfiguration->addMethod("getPrecompiledHeader", &Configuration::getPrecompiledHeader);
	classConfiguration->addMethod("getIncludePaths", &Configuration::getIncludePaths);
	classConfiguration->addMethod("getDefinitions", &Configuration::getDefinitions);
	classConfiguration->addMethod("getLibraryPaths", &Configuration::getLibraryPaths);
	classConfiguration->addMethod("getLibraries", &Configuration::getLibraries);
	classConfiguration->addMethod("getAdditionalCompilerOptions", &Configuration::getAdditionalCompilerOptions);
	classConfiguration->addMethod("getAdditionalLinkerOptions", &Configuration::getAdditionalLinkerOptions);
	classConfiguration->addMethod("getDebugExecutable", &Configuration::getDebugExecutable);
	classConfiguration->addMethod("getDebugArguments", &Configuration::getDebugArguments);
	classConfiguration->addMethod("getDebugEnvironment", &Configuration::getDebugEnvironment);
	classConfiguration->addMethod("getDebugWorkingDirectory", &Configuration::getDebugWorkingDirectory);
	classConfiguration->addMethod("getAggregationItems", &Configuration::getAggregationItems);
	classConfiguration->addMethod("getConsumerLibraryPath", &Configuration::getConsumerLibraryPath);
	m_scriptManager->registerClass(classConfiguration);

	auto classProjectItem = new AutoRuntimeClass< ProjectItem >();
	classProjectItem->addProperty("items", &ProjectItem::getItems);
	classProjectItem->addMethod("getItems", &ProjectItem::getItems);
	m_scriptManager->registerClass(classProjectItem);

	Ref< AutoRuntimeClass< sb::File > > classFile = new AutoRuntimeClass< sb::File >();
	classFile->addProperty("fileName", &sb::File::getFileName);
	classFile->addMethod("getFileName", &sb::File::getFileName);
	classFile->addMethod("getSystemFiles", &File_getSystemFiles);
	m_scriptManager->registerClass(classFile);

	auto classFilter = new AutoRuntimeClass< Filter >();
	classFilter->addProperty("name", &Filter::getName);
	classFilter->addMethod("getName", &Filter::getName);
	m_scriptManager->registerClass(classFilter);

	auto classAggregationItem = new AutoRuntimeClass< AggregationItem >();
	classAggregationItem->addProperty("sourceFile", &AggregationItem::getSourceFile);
	classAggregationItem->addProperty("targetPath", &AggregationItem::getTargetPath);
	classAggregationItem->addMethod("getSourceFile", &AggregationItem::getSourceFile);
	classAggregationItem->addMethod("getTargetPath", &AggregationItem::getTargetPath);
	m_scriptManager->registerClass(classAggregationItem);

	auto classDependency = new AutoRuntimeClass< Dependency >();
	classDependency->addProperty("link", &Dependency_getLink);
	classDependency->addProperty("name", &Dependency::getName);
	classDependency->addProperty("location", &Dependency::getLocation);
	classDependency->addMethod("getLink", &Dependency_getLink);
	classDependency->addMethod("getName", &Dependency::getName);
	classDependency->addMethod("getLocation", &Dependency::getLocation);
	m_scriptManager->registerClass(classDependency);

	auto classExternalDependency = new AutoRuntimeClass< ExternalDependency >();
	classExternalDependency->addProperty("solutionFileName", &ExternalDependency::getSolutionFileName);
	classExternalDependency->addProperty("solution", &ExternalDependency::getSolution);
	classExternalDependency->addProperty("project", &ExternalDependency::getProject);
	classExternalDependency->addMethod("getSolutionFileName", &ExternalDependency::getSolutionFileName);
	classExternalDependency->addMethod("getSolution", &ExternalDependency::getSolution);
	classExternalDependency->addMethod("getProject", &ExternalDependency::getProject);
	m_scriptManager->registerClass(classExternalDependency);

	auto classProjectDependency = new AutoRuntimeClass< ProjectDependency >();
	classProjectDependency->addProperty("project", &ProjectDependency::getProject);
	classProjectDependency->addMethod("getProject", &ProjectDependency::getProject);
	m_scriptManager->registerClass(classProjectDependency);

	auto classHeaderScanner = new AutoRuntimeClass< HeaderScanner >();
	classHeaderScanner->addConstructor();
	classHeaderScanner->addMethod("removeAllIncludePaths", &HeaderScanner::removeAllIncludePaths);
	classHeaderScanner->addMethod("addIncludePath", &HeaderScanner::addIncludePath);
	classHeaderScanner->addMethod("get", &HeaderScanner_get);
	m_scriptManager->registerClass(classHeaderScanner);

	return true;
}

void ScriptProcessor::destroy()
{
	safeDestroy(m_scriptManager);
}

bool ScriptProcessor::prepare(const std::wstring& fileName)
{
	// Read generator script into memory.
	Ref< IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
		return false;

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);
	StringOutputStream ss;

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	file->close();

	// Keep source from stream.
	std::wstring source = ss.str();
	ss.reset();

	// Transform generator script into pure code script.
	size_t offset = 0;

	ss << L"function __main__()" << Endl;

	m_sections.resize(0);
	for (;;)
	{
		size_t s = source.find(L"<?--", offset);
		if (s == source.npos)
			break;

		size_t e = source.find(L"--?>", s);
		if (e == source.npos)
			return false;

		std::wstring section = source.substr(offset, s - offset);
		m_sections.push_back(section);

		ss << L"\toutput:printSection(" << (int32_t)(m_sections.size() - 1) << L")" << Endl;
		ss << source.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	std::wstring section = source.substr(offset);
	m_sections.push_back(section);

	ss << L"\toutput:printSection(" << (int32_t)(m_sections.size() - 1) << L")" << Endl;
	ss << L"end" << Endl;

	// Compile script into blob.
	Ref< script::IScriptBlob > scriptBlob = m_scriptCompiler->compile(fileName, ss.str(), 0);
	if (!scriptBlob)
		return false;

	// Create execution context.
	m_scriptContext = m_scriptManager->createContext(true);
	if (!m_scriptContext)
		return false;

	if (!m_scriptContext->load(scriptBlob))
		return false;

	return true;
}

bool ScriptProcessor::generate(const Solution* solution, const Project* project, const std::wstring& configurationName, const std::wstring& projectPath, std::wstring& output) const
{
	Path projectPathAbs = FileSystem::getInstance().getAbsolutePath(Path(projectPath));

	Ref< Output > o = new Output(m_sections);

	m_scriptContext->setGlobal("output", Any::fromObject(o));
	m_scriptContext->setGlobal("solution", Any::fromObject(const_cast< Solution* >(solution)));
	m_scriptContext->setGlobal("project", Any::fromObject(const_cast< Project* >(project)));
	m_scriptContext->setGlobal("configurationName", Any::fromString(configurationName));
	m_scriptContext->setGlobal("projectPath", Any::fromObject(new Path(projectPathAbs)));
	m_scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));

#if T_PROFILER_ENABLE
	ScriptProfilerListener pl;

	Ref< script::IScriptProfiler > profiler = m_scriptManager->createProfiler();
	if (profiler)
		profiler->addListener(&pl);
#endif

	m_scriptContext->executeFunction("__main__");

#if T_PROFILER_ENABLE
	if (profiler)
	{
		profiler->removeListener(&pl);
		for (auto it : pl.m_durations)
		{
			log::info << it.first << L" : " << (int32_t)(it.second * 1000.0) << L" ms" << Endl;
		}
	}
#endif

	output = o->getProduct();
	return true;
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/CoreClassFactory.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/SafeDestroy.h"
#include "Script/IScriptContext.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Configuration.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/File.h"
#include "SolutionBuilder/Filter.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ProjectItem.h"
#include "SolutionBuilder/Solution.h"

namespace traktor
{
	namespace sb
	{
		namespace
		{

class Output : public Object
{
	T_RTTI_CLASS;

public:
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
		T_ASSERT (id >= 0 && id < int32_t(m_sections.size()));
		m_ss << m_sections[id];
	}

	int32_t addSection(const std::wstring& section)
	{
		m_sections.push_back(section);
		return int32_t(m_sections.size()) - 1;
	}

	std::wstring getProduct()
	{
		return m_ss.str();
	}

private:
	std::vector< std::wstring > m_sections;
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
	for (std::set< Path >::const_iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
		systemFilesOut.push_back(new Path(*i));

	return systemFilesOut;
}

Path FileSystem_getAbsolutePath_1(FileSystem* fileSystem, const Path& relativePath)
{
	return fileSystem->getAbsolutePath(relativePath);
}

Path FileSystem_getAbsolutePath_2(FileSystem* fileSystem, const Path& basePath, const Path& relativePath)
{
	return fileSystem->getAbsolutePath(basePath, relativePath);
}

Ref< Path > FileSystem_getRelativePath(FileSystem* fileSystem, const Path& absolutePath, const Path& relativeToPath)
{
	Path relativePath;
	if (fileSystem->getRelativePath(absolutePath, relativeToPath, relativePath))
		return new Path(relativePath);
	else
		return 0;
}

			}

T_IMPLEMENT_RTTI_CLASS(L"ScriptProcessor", ScriptProcessor, Object)

bool ScriptProcessor::create()
{
	m_scriptManager = new script::ScriptManagerLua();

	BoxesClassFactory().createClasses(m_scriptManager);
	CoreClassFactory().createClasses(m_scriptManager);

	Ref< AutoRuntimeClass< Output > > classOutput = new AutoRuntimeClass< Output >();
	classOutput->addMethod("print", &Output::print);
	classOutput->addMethod("printLn", &Output::printLn);
	classOutput->addMethod("printSection", &Output::printSection);
	m_scriptManager->registerClass(classOutput);

	Ref< AutoRuntimeClass< Solution > > classSolution = new AutoRuntimeClass< Solution >();
	classSolution->addMethod("getName", &Solution::getName);
	classSolution->addMethod("getRootPath", &Solution::getRootPath);
	classSolution->addMethod("getAggregateOutputPath", &Solution::getAggregateOutputPath);
	classSolution->addMethod("getProjects", &Solution::getProjects);
	m_scriptManager->registerClass(classSolution);

	Ref< AutoRuntimeClass< Project > > classProject = new AutoRuntimeClass< Project >();
	classProject->addMethod("getEnable", &Project::getEnable);
	classProject->addMethod("getName", &Project::getName);
	classProject->addMethod("getSourcePath", &Project::getSourcePath);
	classProject->addMethod("getConfigurations", &Project::getConfigurations);
	classProject->addMethod("getConfiguration", &Project::getConfiguration);
	classProject->addMethod("getItems", &Project::getItems);
	classProject->addMethod("getDependencies", &Project::getDependencies);
	m_scriptManager->registerClass(classProject);

	Ref< AutoRuntimeClass< Configuration > > classConfiguration = new AutoRuntimeClass< Configuration >();
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

	Ref< AutoRuntimeClass< ProjectItem > > classProjectItem = new AutoRuntimeClass< ProjectItem >();
	classProjectItem->addMethod("getItems", &ProjectItem::getItems);
	m_scriptManager->registerClass(classProjectItem);

	Ref< AutoRuntimeClass< sb::File > > classFile = new AutoRuntimeClass< sb::File >();
	classFile->addMethod("getFileName", &sb::File::getFileName);
	classFile->addMethod("getSystemFiles", &File_getSystemFiles);
	m_scriptManager->registerClass(classFile);

	Ref< AutoRuntimeClass< Filter > > classFilter = new AutoRuntimeClass< Filter >();
	classFilter->addMethod("getName", &Filter::getName);
	m_scriptManager->registerClass(classFilter);

	Ref< AutoRuntimeClass< AggregationItem > > classAggregationItem = new AutoRuntimeClass< AggregationItem >();
	classAggregationItem->addMethod("getSourceFile", &AggregationItem::getSourceFile);
	classAggregationItem->addMethod("getTargetPath", &AggregationItem::getTargetPath);
	m_scriptManager->registerClass(classAggregationItem);

	Ref< AutoRuntimeClass< Dependency > > classDependency = new AutoRuntimeClass< Dependency >();
	classDependency->addMethod("getLink", &Dependency_getLink);
	classDependency->addMethod("getName", &Dependency::getName);
	classDependency->addMethod("getLocation", &Dependency::getLocation);
	m_scriptManager->registerClass(classDependency);

	Ref< AutoRuntimeClass< ExternalDependency > > classExternalDependency = new AutoRuntimeClass< ExternalDependency >();
	classExternalDependency->addMethod("getSolutionFileName", &ExternalDependency::getSolutionFileName);
	classExternalDependency->addMethod("getSolution", &ExternalDependency::getSolution);
	classExternalDependency->addMethod("getProject", &ExternalDependency::getProject);
	m_scriptManager->registerClass(classExternalDependency);

	Ref< AutoRuntimeClass< ProjectDependency > > classProjectDependency = new AutoRuntimeClass< ProjectDependency >();
	classProjectDependency->addMethod("getProject", &ProjectDependency::getProject);
	m_scriptManager->registerClass(classProjectDependency);

	return true;
}

void ScriptProcessor::destroy()
{
	safeDestroy(m_scriptManager);
}

bool ScriptProcessor::generateFromFile(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& fileName, std::wstring& output) const
{
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

	return generateFromSource(solution, project, projectPath, fileName, ss.str(), output);
}

bool ScriptProcessor::generateFromSource(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& sourceName, const std::wstring& source, std::wstring& output) const
{
	StringOutputStream ss;
	size_t offset = 0;

	Ref< Output > o = new Output();

	ss << L"function __main__()" << Endl;

	for (;;)
	{
		size_t s = source.find(L"<?--", offset);
		if (s == source.npos)
			break;

		size_t e = source.find(L"--?>", s);
		if (e == source.npos)
			return false;

		int32_t id = o->addSection(source.substr(offset, s - offset));
		ss << L"\toutput:printSection(" << id << L")" << Endl;
		ss << source.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	int32_t id = o->addSection(source.substr(offset));
	ss << L"\toutput:printSection(" << id << L")" << Endl;
	ss << L"end" << Endl;

	Ref< script::IScriptBlob > scriptBlob = m_scriptManager->compile(sourceName, ss.str(), 0);
	if (!scriptBlob)
		return false;

	Ref< script::IScriptContext > scriptContext = m_scriptManager->createContext(true);
	if (!scriptContext)
		return false;

	if (!scriptContext->load(scriptBlob))
		return false;

	scriptContext->setGlobal("output", Any::fromObject(o));
	scriptContext->setGlobal("solution", Any::fromObject(const_cast< Solution* >(solution)));
	scriptContext->setGlobal("project", Any::fromObject(const_cast< Project* >(project)));
	scriptContext->setGlobal("projectPath", Any::fromObject(new Path(projectPath)));
	scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	scriptContext->executeFunction("__main__");

	output = o->getProduct();
	return true;
}

	}
}

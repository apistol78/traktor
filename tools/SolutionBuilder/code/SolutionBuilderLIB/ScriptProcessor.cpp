#include <Core/Io/BufferedStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Misc/SafeDestroy.h>
#include <Script/AutoScriptClass.h>
#include <Script/Boxes.h>
#include <Script/IScriptContext.h>
#include <Script/Lua/ScriptManagerLua.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/ScriptProcessor.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ProjectItem.h"
#include "SolutionBuilderLIB/Solution.h"

using namespace traktor;

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

	RefArray< Path > File_getSystemFiles(::File* file, const std::wstring& sourcePath)
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

	Ref< script::AutoScriptClass< Output > > classOutput = new script::AutoScriptClass< Output >();
	classOutput->addMethod(L"print", &Output::print);
	classOutput->addMethod(L"printLn", &Output::printLn);
	classOutput->addMethod(L"printSection", &Output::printSection);
	m_scriptManager->registerClass(classOutput);

	Ref< script::AutoScriptClass< Path > > classPath = new script::AutoScriptClass< Path >();
	classPath->addConstructor();
	classPath->addConstructor< const std::wstring& >();
	classPath->addMethod(L"getOriginal", &Path::getOriginal);
	classPath->addMethod(L"hasVolume", &Path::hasVolume);
	classPath->addMethod(L"getVolume", &Path::getVolume);
	classPath->addMethod(L"isRelative", &Path::isRelative);
	classPath->addMethod(L"getFileName", &Path::getFileName);
	classPath->addMethod(L"getFileNameNoExtension", &Path::getFileNameNoExtension);
	classPath->addMethod(L"getPathOnly", &Path::getPathOnly);
	classPath->addMethod(L"getPathOnlyNoVolume", &Path::getPathOnlyNoVolume);
	classPath->addMethod(L"getPathName", &Path::getPathName);
	classPath->addMethod(L"getPathNameNoExtension", &Path::getPathNameNoExtension);
	classPath->addMethod(L"getPathNameNoVolume", &Path::getPathNameNoVolume);
	classPath->addMethod(L"getExtension", &Path::getExtension);
	classPath->addMethod(L"normalized", &Path::normalized);
	m_scriptManager->registerClass(classPath);

	Ref< script::AutoScriptClass< FileSystem > > classFileSystem = new script::AutoScriptClass< FileSystem >();
	classFileSystem->addMethod(L"exist", &FileSystem::exist);
	classFileSystem->addMethod(L"remove", &FileSystem::remove);
	classFileSystem->addMethod(L"makeDirectory", &FileSystem::makeDirectory);
	classFileSystem->addMethod(L"makeAllDirectories", &FileSystem::makeAllDirectories);
	classFileSystem->addMethod(L"removeDirectory", &FileSystem::removeDirectory);
	classFileSystem->addMethod(L"renameDirectory", &FileSystem::renameDirectory);
	classFileSystem->addMethod(L"getAbsolutePath", &FileSystem_getAbsolutePath_1);
	classFileSystem->addMethod(L"getAbsolutePath", &FileSystem_getAbsolutePath_2);
	classFileSystem->addMethod(L"getRelativePath", &FileSystem_getRelativePath);
	m_scriptManager->registerClass(classFileSystem);

	Ref< script::AutoScriptClass< Solution > > classSolution = new script::AutoScriptClass< Solution >();
	classSolution->addMethod(L"getName", &Solution::getName);
	classSolution->addMethod(L"getRootPath", &Solution::getRootPath);
	classSolution->addMethod(L"getDefinitions", &Solution::getDefinitions);
	classSolution->addMethod(L"getProjects", &Solution::getProjects);
	m_scriptManager->registerClass(classSolution);

	Ref< script::AutoScriptClass< Project > > classProject = new script::AutoScriptClass< Project >();
	classProject->addMethod(L"getEnable", &Project::getEnable);
	classProject->addMethod(L"getName", &Project::getName);
	classProject->addMethod(L"getSourcePath", &Project::getSourcePath);
	classProject->addMethod(L"getConfigurations", &Project::getConfigurations);
	classProject->addMethod(L"getConfiguration", &Project::getConfiguration);
	classProject->addMethod(L"getItems", &Project::getItems);
	classProject->addMethod(L"getDependencies", &Project::getDependencies);
	m_scriptManager->registerClass(classProject);

	Ref< script::AutoScriptClass< Configuration > > classConfiguration = new script::AutoScriptClass< Configuration >();
	classConfiguration->addMethod(L"getName", &Configuration::getName);
	classConfiguration->addMethod(L"getTargetFormat", &Configuration_getTargetFormat);
	classConfiguration->addMethod(L"getTargetProfile", &Configuration_getTargetProfile);
	classConfiguration->addMethod(L"getPrecompiledHeader", &Configuration::getPrecompiledHeader);
	classConfiguration->addMethod(L"getIncludePaths", &Configuration::getIncludePaths);
	classConfiguration->addMethod(L"getDefinitions", &Configuration::getDefinitions);
	classConfiguration->addMethod(L"getLibraryPaths", &Configuration::getLibraryPaths);
	classConfiguration->addMethod(L"getLibraries", &Configuration::getLibraries);
	classConfiguration->addMethod(L"getAdditionalCompilerOptions", &Configuration::getAdditionalCompilerOptions);
	classConfiguration->addMethod(L"getAdditionalLinkerOptions", &Configuration::getAdditionalLinkerOptions);
	m_scriptManager->registerClass(classConfiguration);

	Ref< script::AutoScriptClass< ProjectItem > > classProjectItem = new script::AutoScriptClass< ProjectItem >();
	classProjectItem->addMethod(L"getItems", &ProjectItem::getItems);
	m_scriptManager->registerClass(classProjectItem);

	Ref< script::AutoScriptClass< ::File > > classFile = new script::AutoScriptClass< ::File >();
	classFile->addMethod(L"getFileName", &::File::getFileName);
	classFile->addMethod(L"getSystemFiles", &File_getSystemFiles);
	m_scriptManager->registerClass(classFile);

	Ref< script::AutoScriptClass< Filter > > classFilter = new script::AutoScriptClass< Filter >();
	classFilter->addMethod(L"getName", &Filter::getName);
	m_scriptManager->registerClass(classFilter);

	Ref< script::AutoScriptClass< Dependency > > classDependency = new script::AutoScriptClass< Dependency >();
	classDependency->addMethod(L"shouldLinkWithProduct", &Dependency::shouldLinkWithProduct);
	classDependency->addMethod(L"getName", &Dependency::getName);
	classDependency->addMethod(L"getLocation", &Dependency::getLocation);
	m_scriptManager->registerClass(classDependency);

	Ref< script::AutoScriptClass< ExternalDependency > > classExternalDependency = new script::AutoScriptClass< ExternalDependency >();
	classExternalDependency->addMethod(L"getSolutionFileName", &ExternalDependency::getSolutionFileName);
	classExternalDependency->addMethod(L"getSolution", &ExternalDependency::getSolution);
	classExternalDependency->addMethod(L"getProject", &ExternalDependency::getProject);
	m_scriptManager->registerClass(classExternalDependency);

	Ref< script::AutoScriptClass< ProjectDependency > > classProjectDependency = new script::AutoScriptClass< ProjectDependency >();
	classProjectDependency->addMethod(L"getProject", &ProjectDependency::getProject);
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

	return generateFromSource(solution, project, projectPath, ss.str(), output);
}

bool ScriptProcessor::generateFromSource(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& source, std::wstring& output) const
{
	Ref< script::IScriptContext > scriptContext = m_scriptManager->createContext();
	if (!scriptContext)
		return false;

	Ref< Output > o = new Output();

	scriptContext->setGlobal(L"output", script::Any(o));
	scriptContext->setGlobal(L"solution", script::Any(const_cast< Solution* >(solution)));
	scriptContext->setGlobal(L"project", script::Any(const_cast< Project* >(project)));
	scriptContext->setGlobal(L"projectPath", script::Any(new Path(projectPath)));
	scriptContext->setGlobal(L"fileSystem", script::Any(&FileSystem::getInstance()));

	StringOutputStream ss;
	size_t offset = 0;

	for (;;)
	{
		size_t s = source.find(L"<?--", offset);
		if (s == source.npos)
			break;

		size_t e = source.find(L"--?>", s);
		if (e == source.npos)
			return false;

		int32_t id = o->addSection(source.substr(offset, s - offset));
		ss << L"output:printSection(" << id << L")" << Endl;
		ss << source.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	int32_t id = o->addSection(source.substr(offset));
	ss << L"output:printSection(" << id << L")" << Endl;

	Ref< script::IScriptResource > scriptResource = m_scriptManager->compile(ss.str(), false, 0);
	if (!scriptResource)
		return false;

	if (!scriptContext->executeScript(scriptResource, Guid()))
		return false;

	output = o->getProduct();
	return true;
}

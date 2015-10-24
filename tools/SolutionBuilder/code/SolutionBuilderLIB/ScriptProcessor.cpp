#include <Core/Class/AutoRuntimeClass.h>
#include <Core/Class/Boxes.h>
#include <Core/Io/BufferedStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Misc/SafeDestroy.h>
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

	BoxesClassFactory().createClasses(m_scriptManager);

	Ref< AutoRuntimeClass< Output > > classOutput = new AutoRuntimeClass< Output >();
	classOutput->addMethod("print", &Output::print);
	classOutput->addMethod("printLn", &Output::printLn);
	classOutput->addMethod("printSection", &Output::printSection);
	m_scriptManager->registerClass(classOutput);

	Ref< AutoRuntimeClass< Path > > classPath = new AutoRuntimeClass< Path >();
	classPath->addConstructor();
	classPath->addConstructor< const std::wstring& >();
	classPath->addMethod("getOriginal", &Path::getOriginal);
	classPath->addMethod("hasVolume", &Path::hasVolume);
	classPath->addMethod("getVolume", &Path::getVolume);
	classPath->addMethod("isRelative", &Path::isRelative);
	classPath->addMethod("getFileName", &Path::getFileName);
	classPath->addMethod("getFileNameNoExtension", &Path::getFileNameNoExtension);
	classPath->addMethod("getPathOnly", &Path::getPathOnly);
	classPath->addMethod("getPathOnlyNoVolume", &Path::getPathOnlyNoVolume);
	classPath->addMethod("getPathName", &Path::getPathName);
	classPath->addMethod("getPathNameNoExtension", &Path::getPathNameNoExtension);
	classPath->addMethod("getPathNameNoVolume", &Path::getPathNameNoVolume);
	classPath->addMethod("getExtension", &Path::getExtension);
	classPath->addMethod("normalized", &Path::normalized);
	m_scriptManager->registerClass(classPath);

	Ref< AutoRuntimeClass< FileSystem > > classFileSystem = new AutoRuntimeClass< FileSystem >();
	classFileSystem->addMethod("exist", &FileSystem::exist);
	classFileSystem->addMethod("remove", &FileSystem::remove);
	classFileSystem->addMethod("makeDirectory", &FileSystem::makeDirectory);
	classFileSystem->addMethod("makeAllDirectories", &FileSystem::makeAllDirectories);
	classFileSystem->addMethod("removeDirectory", &FileSystem::removeDirectory);
	classFileSystem->addMethod("renameDirectory", &FileSystem::renameDirectory);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_1);
	classFileSystem->addMethod("getAbsolutePath", &FileSystem_getAbsolutePath_2);
	classFileSystem->addMethod("getRelativePath", &FileSystem_getRelativePath);
	m_scriptManager->registerClass(classFileSystem);

	Ref< AutoRuntimeClass< Solution > > classSolution = new AutoRuntimeClass< Solution >();
	classSolution->addMethod("getName", &Solution::getName);
	classSolution->addMethod("getRootPath", &Solution::getRootPath);
	classSolution->addMethod("getDefinitions", &Solution::getDefinitions);
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
	m_scriptManager->registerClass(classConfiguration);

	Ref< AutoRuntimeClass< ProjectItem > > classProjectItem = new AutoRuntimeClass< ProjectItem >();
	classProjectItem->addMethod("getItems", &ProjectItem::getItems);
	m_scriptManager->registerClass(classProjectItem);

	Ref< AutoRuntimeClass< ::File > > classFile = new AutoRuntimeClass< ::File >();
	classFile->addMethod("getFileName", &::File::getFileName);
	classFile->addMethod("getSystemFiles", &File_getSystemFiles);
	m_scriptManager->registerClass(classFile);

	Ref< AutoRuntimeClass< Filter > > classFilter = new AutoRuntimeClass< Filter >();
	classFilter->addMethod("getName", &Filter::getName);
	m_scriptManager->registerClass(classFilter);

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

	return generateFromSource(solution, project, projectPath, ss.str(), output);
}

bool ScriptProcessor::generateFromSource(const Solution* solution, const Project* project, const std::wstring& projectPath, const std::wstring& source, std::wstring& output) const
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

	Ref< script::IScriptBlob > scriptBlob = m_scriptManager->compile(L"", ss.str(), 0);
	if (!scriptBlob)
		return false;

	Ref< script::IScriptContext > scriptContext = m_scriptManager->createContext();
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

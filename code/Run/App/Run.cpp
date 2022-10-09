#include "Compress/ClassFactory.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/BoxedClassFactory.h"
#include "Core/Class/CastAny.h"
#include "Core/Class/CoreClassFactory1.h"
#include "Core/Class/CoreClassFactory2.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Core/System/ResolveEnv.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/DrawingClassFactory.h"
#include "Json/JsonClassFactory.h"
#include "Net/NetClassFactory.h"
#include "Run/App/ProduceOutput.h"
#include "Run/App/Run.h"
#include "Run/App/StdOutput.h"
#include "Run/App/StreamInput.h"
#include "Run/App/StreamOutput.h"
#include "Script/IScriptCompiler.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptManager.h"
#include "Sql/SqlClassFactory.h"
#include "Sql/Sqlite3/Sqlite3ClassFactory.h"
#include "Xml/XmlClassFactory.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace run
	{
		namespace
		{

void IOutput_printLn_0(IOutput* self)
{
	self->printLn(L"");
}

void IOutput_printLn_1(IOutput* self, const std::wstring& s)
{
	self->printLn(s);
}

int32_t Run_run_1(Run* self, const std::wstring& command)
{
	return self->run(command);
}

int32_t Run_run_2(Run* self, const std::wstring& command, const Any& saveOutputAs)
{
	return self->run(command, saveOutputAs.isString() ? saveOutputAs.getWideString() : L"(null)");
}

int32_t Run_run_3(Run* self, const std::wstring& command, const Any& saveOutputAs, const Environment* env)
{
	return self->run(command, saveOutputAs.isString() ? saveOutputAs.getWideString() : L"(null)", env);
}

int32_t Run_execute_1(Run* self, const std::wstring& command)
{
	return self->execute(command);
}

int32_t Run_execute_2(Run* self, const std::wstring& command, const Any& saveOutputAs)
{
	return self->execute(command, saveOutputAs.isString() ? saveOutputAs.getWideString() : L"(null)");
}

int32_t Run_execute_3(Run* self, const std::wstring& command, const Any& saveOutputAs, const Environment* env)
{
	return self->execute(command, saveOutputAs.isString() ? saveOutputAs.getWideString() : L"(null)", env);
}

Any Run_getProperty_1(Run* self, const std::wstring& fileName, const std::wstring& propertyName, const Any& defaultValue)
{
	return self->getProperty(fileName, std::wstring(), propertyName, defaultValue);
}

Any Run_getProperty_2(Run* self, const std::wstring& fileName1, const std::wstring& fileName2, const std::wstring& propertyName, const Any& defaultValue)
{
	return self->getProperty(fileName1, fileName2, propertyName, defaultValue);
}

Ref< PropertyGroup > loadSettings(const std::wstring& settingsFile)
{
	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
		return xml::XmlDeserializer(file, settingsFile).readObject< PropertyGroup >();
	else
		return nullptr;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.run.Run", Run, Object)

Run::Run(
	script::IScriptCompiler* scriptCompiler,
	script::IScriptManager* scriptManager,
	script::IScriptContext* scriptContext
)
:	m_scriptCompiler(scriptCompiler)
,	m_scriptManager(scriptManager)
,	m_scriptContext(scriptContext)
,	m_exitCode(0)
{
	m_cwd.push_back(L"");
}

void Run::cd(const std::wstring& path)
{
	Path tmp = Path(m_cwd.back()) + Path(path);
	m_cwd.back() = tmp.getPathNameOS();
}

void Run::pushd(const std::wstring& path)
{
	Path tmp = Path(m_cwd.back()) + Path(path);
	m_cwd.push_back(tmp.getPathNameOS());
}

void Run::popd()
{
	m_cwd.pop_back();
	if (m_cwd.empty())
		m_cwd.push_back(L"");
}

std::wstring Run::cwd() const
{
	return m_cwd.back();
}

int32_t Run::run(const std::wstring& command, const std::wstring& saveOutputAs, const Environment* env)
{
	Path executable = OS::getInstance().getExecutable();
	return execute(L"\"" + executable.getPathName() + L"\" " + command, saveOutputAs, env);
}

int32_t Run::execute(const std::wstring& command, const std::wstring& saveOutputAs, const Environment* env)
{
	Path workingDirectory = FileSystem::getInstance().getAbsolutePath(cwd());

	Ref< IProcess > process = OS::getInstance().execute(
		command,
		workingDirectory,
		env,
		OS::EfMute // OS::EfRedirectStdIO | OS::EfMute
	);
	if (!process)
		return -1;

	PipeReader stdOutReader(
		process->getPipeStream(IProcess::SpStdOut)
	);
	PipeReader stdErrReader(
		process->getPipeStream(IProcess::SpStdErr)
	);

	Ref< FileOutputStream > fileOutput;
	bool nullOutput = false;

	if (!saveOutputAs.empty())
	{
		if (saveOutputAs != L"(null)")
		{
			Ref< traktor::IStream > output = FileSystem::getInstance().open(saveOutputAs, File::FmWrite);
			if (!output)
				return -2;

			fileOutput = new FileOutputStream(output, new Utf8Encoding());
		}
		else
			nullOutput = true;
	}

	StringOutputStream stdOut;
	StringOutputStream stdErr;
	std::wstring str;
	while (!process->wait(0))
	{
		auto pipe = process->waitPipeStream(100);
		if (pipe == process->getPipeStream(IProcess::SpStdOut))
		{
			PipeReader::Result result;
			while ((result = stdOutReader.readLine(str)) == PipeReader::RtOk)
			{
				if (fileOutput)
					(*fileOutput) << str << Endl;
				else if (!nullOutput)
					log::info << str << Endl;
				stdOut << str << Endl;
			}
		}
		else if (pipe == process->getPipeStream(IProcess::SpStdErr))
		{
			PipeReader::Result result;
			while ((result = stdErrReader.readLine(str)) == PipeReader::RtOk)
			{
				if (!nullOutput)
					log::error << str << Endl;
				stdErr << str << Endl;
			}
		}
	}

	safeClose(fileOutput);

	m_stdOut = stdOut.str();
	m_stdErr = stdErr.str();
	m_exitCode = process->exitCode();

	return m_exitCode;
}

const std::wstring& Run::stdOut() const
{
	return m_stdOut;
}

const std::wstring& Run::stdErr() const
{
	return m_stdErr;
}

int32_t Run::exitCode() const
{
	return m_exitCode;
}

bool Run::exist(const std::wstring& path)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), path);
	return FileSystem::getInstance().exist(sourcePath);
}

bool Run::rm(const std::wstring& path)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), path);

	RefArray< File > sourceFiles;
	FileSystem::getInstance().find(sourcePath, sourceFiles);
	for (auto sourceFile : sourceFiles)
	{
		if (!FileSystem::getInstance().remove(sourceFile->getPath()))
			return false;
	}

	return true;
}

bool Run::copy(const std::wstring& source, const std::wstring& target)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), source);
	Ref< File > targetFile = FileSystem::getInstance().get(target);

	RefArray< File > sourceFiles;
	FileSystem::getInstance().find(sourcePath, sourceFiles);

	for (auto sourceFile : sourceFiles)
	{
		if (!sourceFile->isDirectory())
		{
			// If target specifies an existing directory then copy file into this directory.
			std::wstring targetFileName = target;
			if (targetFile && targetFile->isDirectory())
				targetFileName = target + L"/" + sourceFile->getPath().getFileName();

			if (!FileSystem::getInstance().copy(targetFileName, sourceFile->getPath(), false))
				return false;
		}
		else
		{
			// Source file is a directory; recursively copy directory.
			std::wstring sourceDirectory = sourceFile->getPath().getFileName();
			if (sourceDirectory != L"." && sourceDirectory != L"..")
			{
				if (!FileSystem::getInstance().makeDirectory(target + L"/" + sourceDirectory))
					return false;

				if (!copy(sourceFile->getPath().getPathName() + L"/*", target + L"/" + sourceDirectory))
					return false;
			}
		}
	}

	return true;
}

bool Run::replace(const std::wstring& source, const std::wstring& target)
{
	Path sourcePath = FileSystem::getInstance().getAbsolutePath(cwd(), source);
	Ref< File > targetFile = FileSystem::getInstance().get(target);

	RefArray< File > sourceFiles;
	FileSystem::getInstance().find(sourcePath, sourceFiles);

	for (auto sourceFile : sourceFiles)
	{
		if (!sourceFile->isDirectory())
		{
			// If target specifies an existing directory then copy file into this directory.
			std::wstring targetFileName = target;
			if (targetFile && targetFile->isDirectory())
				targetFileName = target + L"/" + sourceFile->getPath().getFileName();

			if (!FileSystem::getInstance().copy(targetFileName, sourceFile->getPath(), true))
				return false;
		}
		else
		{
			// Source file is a directory; recursively copy directory.
			std::wstring sourceDirectory = sourceFile->getPath().getFileName();
			if (sourceDirectory != L"." && sourceDirectory != L"..")
			{
				if (!FileSystem::getInstance().makeDirectory(target + L"/" + sourceDirectory))
					return false;

				if (!replace(sourceFile->getPath().getPathName() + L"/*", target + L"/" + sourceDirectory))
					return false;
			}
		}
	}

	return true;
}

bool Run::mkdir(const std::wstring& path)
{
	Path fullPath = FileSystem::getInstance().getAbsolutePath(cwd(), path);
	return FileSystem::getInstance().makeAllDirectories(fullPath);
}

bool Run::rmdir(const std::wstring& path)
{
	Path fullPath = FileSystem::getInstance().getAbsolutePath(cwd(), path);
	return FileSystem::getInstance().removeDirectory(fullPath);
}

void Run::sleep(int32_t ms)
{
	ThreadManager::getInstance().getCurrentThread()->sleep(ms);
}

Any Run::getProperty(const std::wstring& fileName1, const std::wstring& fileName2, const std::wstring& propertyName, const Any& defaultValue) const
{
	Ref< PropertyGroup > p = loadSettings(fileName1);
	if (!p)
		return Any();

	if (!fileName2.empty())
	{
		Ref< PropertyGroup > pr = loadSettings(fileName2);
		if (!pr)
			return Any();

		if ((p = p->merge(pr, PropertyGroup::MmReplace)) == nullptr)
			return Any();
	}

	Ref< IPropertyValue > property = p->getProperty(propertyName);
	if (!property)
		return Any();

	if (const PropertyBoolean* propertyBoolean = dynamic_type_cast< const PropertyBoolean* >(property))
		return Any::fromBoolean(*propertyBoolean);
	else if (const PropertyInteger* propertyInteger = dynamic_type_cast< const PropertyInteger* >(property))
		return Any::fromInt32(*propertyInteger);
	else if (const PropertyFloat* propertyFloat = dynamic_type_cast< const PropertyFloat* >(property))
		return Any::fromFloat(*propertyFloat);
	else if (const PropertyString* propertyString = dynamic_type_cast< const PropertyString* >(property))
		return Any::fromString(*propertyString);
	else
		return Any::fromObject(property);
}

bool Run::setProperty(const std::wstring& fileName, const std::wstring& propertyName, const Any& value) const
{
	Ref< PropertyGroup > p = loadSettings(fileName);
	if (!p)
		p = new PropertyGroup();

	if (value.isBoolean())
		p->setProperty< PropertyBoolean >(propertyName, value.getBoolean());
	else if (value.isInt32())
		p->setProperty< PropertyInteger >(propertyName, value.getInt32());
	else if (value.isInt64())
		p->setProperty< PropertyInteger >(propertyName, (int32_t)value.getInt64());
	else if (value.isFloat())
		p->setProperty< PropertyFloat >(propertyName, value.getFloat());
	else if (value.isString())
		p->setProperty< PropertyString >(propertyName, value.getWideString());
	else if (is_a< IPropertyValue >(value.getObject()))
		p->setProperty(propertyName, static_cast< IPropertyValue* >(value.getObjectUnsafe()));
	else
		return false;

	Ref< IStream > f = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (f)
		return xml::XmlSerializer(f).writeObject(p);
	else
		return false;
}

std::wstring Run::resolve(const std::wstring& text)
{
	return resolveEnv(text, OS::getInstance().getEnvironment());
}

bool Run::loadModule(const std::wstring& moduleName)
{
	Library library;
	if (!library.open(Path(moduleName)))
		return false;

	// Re-register all runtime classes in-case a new factory has been loaded.
	registerRuntimeClasses(m_scriptManager);
	library.detach();
	return true;
}

bool Run::loadScript(const std::wstring& fileName)
{
	Path pathName(fileName);
	if (pathName.isRelative() && !cwd().empty())
		pathName = cwd() + L"/" + pathName.getPathName();

	Ref< IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to open file \"" << pathName.getPathName() << L"\"; no such file." << Endl;
		return false;
	}

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);
	StringOutputStream ss;

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	safeClose(file);

	Ref< script::IScriptBlob > blob = m_scriptCompiler->compile(
		fileName,
		ss.str(),
		nullptr
	);
	if (!blob)
		return false;

	return m_scriptContext->load(blob);
}

std::wstring Run::evaluate(const std::wstring& fileName)
{
	StringOutputStream ss;

	// Read entire script into memory.
	Path pathName(fileName);
	if (pathName.isRelative() && !cwd().empty())
		pathName = cwd() + L"/" + pathName.getPathName();

	Ref< IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to open file \"" << pathName.getPathName() << L"\"; no such file." << Endl;
		return L"";
	}

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	safeClose(file);

	std::wstring text = ss.str();
	ss.reset();

	// Create template script function and load into context.
	Ref< ProduceOutput > o = new ProduceOutput();

	ss << L"function __evaluate__(output)" << Endl;
	size_t offset = 0;
	for (;;)
	{
		size_t s = text.find(L"<!--", offset);
		if (s == text.npos)
			break;

		size_t e = text.find(L"--!>", s);
		if (e == text.npos)
		{
			log::error << L"Template syntax error; missing end." << Endl;
			return L"";
		}

		int32_t id = o->addSection(text.substr(offset, s - offset));
		ss << L"\toutput:printSection(" << id << L")" << Endl;
		ss << text.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}
	int32_t id = o->addSection(text.substr(offset));
	ss << L"\toutput:printSection(" << id << L")" << Endl;
	ss << L"end" << Endl;

	Ref< script::IScriptBlob > scriptBlob = m_scriptCompiler->compile(fileName, ss.str(), nullptr);
	if (!scriptBlob)
	{
		log::error << L"Unable to compile script." << Endl;
		return L"";
	}

	m_scriptContext->load(scriptBlob);

	// Execute function and return result.
	Any argv[] = { Any::fromObject(o) };
	m_scriptContext->executeFunction("__evaluate__", sizeof_array(argv), argv);

	return o->getProduct();
}

void Run::registerRuntimeClasses(script::IScriptManager* scriptManager)
{
	OrderedClassRegistrar registrar;

	// System classes.
	T_FORCE_LINK_REF(BoxedClassFactory);
	T_FORCE_LINK_REF(CoreClassFactory1);
	T_FORCE_LINK_REF(CoreClassFactory2);
	T_FORCE_LINK_REF(compress::ClassFactory);
	T_FORCE_LINK_REF(drawing::DrawingClassFactory);
	T_FORCE_LINK_REF(json::JsonClassFactory);
	T_FORCE_LINK_REF(net::NetClassFactory);
	T_FORCE_LINK_REF(xml::XmlClassFactory);
	T_FORCE_LINK_REF(sql::SqlClassFactory);
	T_FORCE_LINK_REF(sql::Sqlite3ClassFactory);

	// Register all runtime classes, first collect all classes
	// and then register them in class dependency order.
	for (const auto runtimeClassFactoryType : type_of< IRuntimeClassFactory >().findAllOf(false))
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >(runtimeClassFactoryType->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}

	// IOutput
	auto classIOutput = new AutoRuntimeClass< IOutput >();
	classIOutput->addMethod("print", &IOutput::print);
	classIOutput->addMethod("printLn", &IOutput_printLn_0);
	classIOutput->addMethod("printLn", &IOutput_printLn_1);
	registrar.registerClass(classIOutput);

	// ProduceOutput
	auto classProduceOutput = new AutoRuntimeClass< ProduceOutput >();
	classProduceOutput->addMethod("printSection", &ProduceOutput::printSection);
	classProduceOutput->addMethod("addSection", &ProduceOutput::addSection);
	registrar.registerClass(classProduceOutput);

	// StdOutput
	auto classStdOutput = new AutoRuntimeClass< StdOutput >();
	registrar.registerClass(classStdOutput);

	// StreamOutput
	auto classStreamOutput = new AutoRuntimeClass< StreamOutput >();
	classStreamOutput->addConstructor< traktor::IStream*, IEncoding* >();
	classStreamOutput->addConstructor< traktor::IStream*, IEncoding*, const std::wstring& >();
	registrar.registerClass(classStreamOutput);

	// IInput
	auto classIInput = new AutoRuntimeClass< IInput >();
	classIInput->addProperty("endOfFile", &IInput::endOfFile);
	classIInput->addMethod("endOfFile", &IInput::endOfFile);
	classIInput->addMethod("readChar", &IInput::readChar);
	classIInput->addMethod("readLn", &IInput::readLn);
	registrar.registerClass(classIInput);

	// StreamInput
	auto classStreamInput = new AutoRuntimeClass< StreamInput >();
	classStreamInput->addConstructor< traktor::IStream*, IEncoding* >();
	registrar.registerClass(classStreamInput);

	// Run
	auto classRun = new AutoRuntimeClass< Run >();
	classRun->addProperty("cwd", &Run::cwd);
	classRun->addProperty("stdOut", &Run::stdOut);
	classRun->addProperty("stdErr", &Run::stdErr);
	classRun->addProperty("exitCode", &Run::exitCode);
	classRun->addMethod("cd", &Run::cd);
	classRun->addMethod("pushd", &Run::pushd);
	classRun->addMethod("popd", &Run::popd);
	classRun->addMethod("cwd", &Run::cwd);
	classRun->addMethod("run", &Run_run_1);
	classRun->addMethod("run", &Run_run_2);
	classRun->addMethod("run", &Run_run_3);
	classRun->addMethod("execute", &Run_execute_1);
	classRun->addMethod("execute", &Run_execute_2);
	classRun->addMethod("execute", &Run_execute_3);
	classRun->addMethod("stdOut", &Run::stdOut);
	classRun->addMethod("stdErr", &Run::stdErr);
	classRun->addMethod("exitCode", &Run::exitCode);
	classRun->addMethod("exist", &Run::exist);
	classRun->addMethod("rm", &Run::rm);
	classRun->addMethod("copy", &Run::copy);
	classRun->addMethod("replace", &Run::replace);
	classRun->addMethod("mkdir", &Run::mkdir);
	classRun->addMethod("rmdir", &Run::rmdir);
	classRun->addMethod("sleep", &Run::sleep);
	classRun->addMethod("getProperty", &Run_getProperty_1);
	classRun->addMethod("getProperty", &Run_getProperty_2);
	classRun->addMethod("setProperty", &Run::setProperty);
	classRun->addMethod("resolve", &Run::resolve);
	classRun->addMethod("loadModule", &Run::loadModule);
	classRun->addMethod("loadScript", &Run::loadScript);
	classRun->addMethod("evaluate", &Run::evaluate);
	registrar.registerClass(classRun);

	// Register all classes to script manager; in class hierarchy order.
	registrar.registerClassesInOrder(scriptManager);
}

	}
}

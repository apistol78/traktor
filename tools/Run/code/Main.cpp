#include <Core/Class/AutoRuntimeClass.h>
#include <Core/Class/Boxes.h>
#include <Core/Class/CoreClassFactory.h>
#include <Core/Class/OrderedClassRegistrar.h>
#include <Core/Io/BufferedStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Misc/SafeDestroy.h>
#include <Drawing/DrawingClassFactory.h>
#include <Net/NetClassFactory.h>
#include <Net/Network.h>
#include <Script/IScriptContext.h>
#include <Script/Lua/ScriptManagerLua.h>
#include <Sql/SqlClassFactory.h>
#include <Sql/Sqlite3/Sqlite3ClassFactory.h>
#include <Xml/XmlClassFactory.h>
#include "Environment.h"
#include "ProduceOutput.h"
#include "Run.h"
#include "StdOutput.h"
#include "StreamInput.h"
#include "StreamOutput.h"

using namespace traktor;

int32_t Run_run_1(Run* self, const std::wstring& command)
{
	return self->run(command);
}

int32_t Run_run_2(Run* self, const std::wstring& command, const std::wstring& saveOutputAs)
{
	return self->run(command, saveOutputAs);
}

int32_t Run_execute_1(Run* self, const std::wstring& command)
{
	return self->execute(command);
}

int32_t Run_execute_2(Run* self, const std::wstring& command, const std::wstring& saveOutputAs)
{
	return self->execute(command, saveOutputAs);
}


Ref< script::IScriptManager > createScriptManager()
{
	Ref< script::IScriptManager > scriptManager = new script::ScriptManagerLua();

	OrderedClassRegistrar registrar;

	// System classes.
	BoxesClassFactory().createClasses(&registrar);
	CoreClassFactory().createClasses(&registrar);
	drawing::DrawingClassFactory().createClasses(&registrar);
	xml::XmlClassFactory().createClasses(&registrar);
	net::NetClassFactory().createClasses(&registrar);
	sql::SqlClassFactory().createClasses(&registrar);
	sql::Sqlite3ClassFactory().createClasses(&registrar);

	// IOutput
	Ref< AutoRuntimeClass< IOutput > > classIOutput = new AutoRuntimeClass< IOutput >();
	classIOutput->addMethod("print", &IOutput::print);
	classIOutput->addMethod("printLn", &IOutput::printLn);
	registrar.registerClass(classIOutput);

	// ProduceOutput
	Ref< AutoRuntimeClass< ProduceOutput > > classProduceOutput = new AutoRuntimeClass< ProduceOutput >();
	classProduceOutput->addMethod("printSection", &ProduceOutput::printSection);
	classProduceOutput->addMethod("addSection", &ProduceOutput::addSection);
	registrar.registerClass(classProduceOutput);

	// StdOutput
	Ref< AutoRuntimeClass< StdOutput > > classStdOutput = new AutoRuntimeClass< StdOutput >();
	registrar.registerClass(classStdOutput);

	// StreamOutput
	Ref< AutoRuntimeClass< StreamOutput > > classStreamOutput = new AutoRuntimeClass< StreamOutput >();
	classStreamOutput->addConstructor< traktor::IStream*, IEncoding* >();
	classStreamOutput->addConstructor< traktor::IStream*, IEncoding*, const std::wstring& >();
	registrar.registerClass(classStreamOutput);

	// IInput
	Ref< AutoRuntimeClass< IInput > > classIInput = new AutoRuntimeClass< IInput >();
	classIInput->addMethod("endOfFile", &IInput::endOfFile);
	classIInput->addMethod("readChar", &IInput::readChar);
	classIInput->addMethod("readLn", &IInput::readLn);
	registrar.registerClass(classIInput);

	// StreamInput
	Ref< AutoRuntimeClass< StreamInput > > classStreamInput = new AutoRuntimeClass< StreamInput >();
	classStreamInput->addConstructor< traktor::IStream*, IEncoding* >();
	registrar.registerClass(classStreamInput);

	// Environment
	Ref< AutoRuntimeClass< Environment > > classEnvironment = new AutoRuntimeClass< Environment >();
	classEnvironment->addConstructor();
	classEnvironment->addMethod("set", &Environment::set);
	classEnvironment->addMethod("get", &Environment::get);
	registrar.registerClass(classEnvironment);

	// Run
	Ref< AutoRuntimeClass< Run > > classRun = new AutoRuntimeClass< Run >();
	classRun->addConstructor();
	classRun->addMethod("cd", &Run::cd);
	classRun->addMethod("pushd", &Run::pushd);
	classRun->addMethod("popd", &Run::popd);
	classRun->addMethod("cwd", &Run::cwd);
	classRun->addMethod("run", &Run_run_1);
	classRun->addMethod("run", &Run_run_2);
	classRun->addMethod("execute", &Run_execute_1);
	classRun->addMethod("execute", &Run_execute_2);
	classRun->addMethod("stdOut", &Run::stdOut);
	classRun->addMethod("stdErr", &Run::stdErr);
	classRun->addMethod("exitCode", &Run::exitCode);
	classRun->addMethod("copy", &Run::copy);
	classRun->addMethod("replace", &Run::replace);
	classRun->addMethod("mkdir", &Run::mkdir);
	classRun->addMethod("rmdir", &Run::rmdir);
	classRun->addMethod("sleep", &Run::sleep);
	registrar.registerClass(classRun);

	// Register all classes to script manager; in class hierarchy order.
	registrar.registerClassesInOrder(scriptManager);
	return scriptManager;
}

int32_t executeRun(script::IScriptManager* scriptManager, const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	// Compile script into a runnable resource.
	Ref< script::IScriptResource > scriptResource = scriptManager->compile(fileName.getPathName(), text, 0, 0);
	if (!scriptResource)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Create an empty context which we use as a template for actual script context;
	// because we want globals to be accessible at the creation of the script context.
	Ref< script::IScriptContext > globalContext = scriptManager->createContext(0, 0);
	if (!globalContext)
	{
		log::error << L"Unable to create global script context" << Endl;
		return 1;
	}

	Ref< Environment > environment = new Environment(OS::getInstance().getEnvironment());
	environment->set(L"RUN_SCRIPT", fileName.getPathName());
	globalContext->setGlobal("environment", Any::fromObject(environment));
	globalContext->setGlobal("run", Any::fromObject(new Run()));
	globalContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	globalContext->setGlobal("os", Any::fromObject(&OS::getInstance()));
	globalContext->setGlobal("stdout", Any::fromObject(new StdOutput(stdout)));
	globalContext->setGlobal("stderr", Any::fromObject(new StdOutput(stderr)));

	// Create script context; some scripts might actually do it's work while
	// being constructed which is perfectly valid.
	Ref< script::IScriptContext > scriptContext = scriptManager->createContext(scriptResource, globalContext);
	if (!scriptContext)
		return 1;

	// Execute optional "main" function.
	Any retval = Any::fromInteger(0);
	if (scriptContext->haveFunction("main"))
	{
		// Transform arguments into script array.
		std::vector< std::wstring > args;
		for (int32_t i = 1; i < cmdLine.getCount(); ++i)
			args.push_back(cmdLine.getString(i));

		Any a = CastAny< std::vector< std::wstring > >::set(args);
		retval = scriptContext->executeFunction("main", 1, &a);
	}

	safeDestroy(scriptContext);
	safeDestroy(globalContext);

	return retval.getInteger();
}

int32_t executeTemplate(script::IScriptManager* scriptManager, const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	Ref< ProduceOutput > o = new ProduceOutput();

	// Transform into valid script.
	StringOutputStream ss;
	size_t offset = 0;

	for (;;)
	{
		size_t s = text.find(L"<!--", offset);
		if (s == text.npos)
			break;

		size_t e = text.find(L"--!>", s);
		if (e == text.npos)
		{
			log::error << L"Template syntax error; missing end" << Endl;
			return 1;
		}

		int32_t id = o->addSection(text.substr(offset, s - offset));
		ss << L"output:printSection(" << id << L")" << Endl;
		ss << text.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	int32_t id = o->addSection(text.substr(offset));
	ss << L"output:printSection(" << id << L")" << Endl;

	Ref< script::IScriptResource > scriptResource = scriptManager->compile(fileName.getPathName(), ss.str(), 0, 0);
	if (!scriptResource)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Create an empty context which we use as a template for actual script context;
	// because we want globals to be accessible at the creation of the script context.
	Ref< script::IScriptContext > globalContext = scriptManager->createContext(0, 0);
	if (!globalContext)
	{
		log::error << L"Unable to create global script context" << Endl;
		return 1;
	}

	// Transform arguments into script array.
	std::vector< std::wstring > args;
	for (int32_t i = 1; i < cmdLine.getCount(); ++i)
		args.push_back(cmdLine.getString(i));

	Ref< Environment > environment = new Environment(OS::getInstance().getEnvironment());
	environment->set(L"RUN_SCRIPT", fileName.getPathName());
	globalContext->setGlobal("environment", Any::fromObject(environment));
	globalContext->setGlobal("run", Any::fromObject(new Run()));
	globalContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	globalContext->setGlobal("os", Any::fromObject(&OS::getInstance()));
	globalContext->setGlobal("stdout", Any::fromObject(new StdOutput(stdout)));
	globalContext->setGlobal("stderr", Any::fromObject(new StdOutput(stderr)));
	globalContext->setGlobal("output", Any::fromObject(o));
	globalContext->setGlobal("args", CastAny< std::vector< std::wstring > >::set(args));

	Ref< script::IScriptContext > scriptContext = scriptManager->createContext(scriptResource, globalContext);
	if (!scriptContext)
		return 1;

	safeDestroy(scriptContext);
	safeDestroy(globalContext);

	log::info << o->getProduct() << Endl;

	return 0;
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Run 1.4" << Endl;
		log::info << Endl;
		log::info << L"Usage: Run (option(s)) [<file>.run|<file>.template] (args ...)" << Endl;
		log::info << Endl;
		log::info << L"  Options:" << Endl;
		log::info << L"    -as-run                   Run file as run" << Endl;
		log::info << L"    -as-template              Run file as template" << Endl;
		log::info << Endl;
		log::info << L"  For .run files:" << Endl;
		log::info << L"    -e,-entry-point=function  Script entry point (default \"main\")" << Endl;
		log::info << Endl;
		log::info << L"  For .template files:" << Endl;
		log::info << L"    -o,-output=filename       Output file (default stdout)" << Endl;
		return 0;
	}

	Path fileName = cmdLine.getString(0);

	Ref< traktor::IStream > file = FileSystem::getInstance().open(fileName, traktor::File::FmRead);
	if (!file)
	{
		log::error << L"Failed to open \"" << fileName.getPathName() << L"\"" << Endl;
		return 1;
	}

	Utf8Encoding encoding;
	BufferedStream stream(file);
	StringReader reader(&stream, &encoding);
	StringOutputStream ss;

	std::wstring tmp;
	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	safeClose(file);

	std::wstring text = ss.str();

	Ref< script::IScriptManager > scriptManager = createScriptManager();
	if (!scriptManager)
	{
		log::error << L"Failed to create script manager" << Endl;
		return 1;
	}

	net::Network::initialize();

	int32_t result = 1;

	bool explicitRun = cmdLine.hasOption(L"as-run");
	bool explicitTemplate = cmdLine.hasOption(L"as-template");

	if ((explicitRun && !explicitTemplate) || compareIgnoreCase< std::wstring >(fileName.getExtension(), L"run") == 0)
		result = executeRun(scriptManager, text, fileName, cmdLine);
	else if ((!explicitRun && explicitTemplate) || compareIgnoreCase< std::wstring >(fileName.getExtension(), L"template") == 0)
		result = executeTemplate(scriptManager, text, fileName, cmdLine);
	else
		log::error << L"Unknown file type \"" << fileName.getExtension() << L"\"; must be either \"run\" or \"template\"" << Endl;

	net::Network::finalize();

	safeDestroy(scriptManager);
	return result;
}

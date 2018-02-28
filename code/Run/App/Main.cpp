/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/CoreClassFactory.h"
#include "Core/Class/CoreClassFactory2.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/System/Environment.h"
#include "Core/System/OS.h"
#include "Drawing/DrawingClassFactory.h"
#include "Net/NetClassFactory.h"
#include "Net/Network.h"
#include "Run/App/ProduceOutput.h"
#include "Run/App/Run.h"
#include "Run/App/StdOutput.h"
#include "Run/App/StreamInput.h"
#include "Run/App/StreamOutput.h"
#include "Script/IScriptContext.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Sql/SqlClassFactory.h"
#include "Sql/Sqlite3/Sqlite3ClassFactory.h"
#include "Xml/XmlClassFactory.h"

namespace
{
		
traktor::Ref< traktor::script::IScriptManager > g_scriptManager;		

}

namespace traktor
{
	namespace run
	{

void registerRuntimeClasses();

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

bool Run_loadModule(Run* self, const std::wstring& moduleName)
{
	Library library;
	if (!library.open(Path(moduleName)))
		return false;

	// Re-register all runtime classes in-case a new factory has been loaded.
	registerRuntimeClasses();

	library.detach();
	return true;
}

void registerRuntimeClasses()
{
	OrderedClassRegistrar registrar;

	// System classes.
	T_FORCE_LINK_REF(CoreClassFactory);
	T_FORCE_LINK_REF(CoreClassFactory2);
	T_FORCE_LINK_REF(drawing::DrawingClassFactory);
	T_FORCE_LINK_REF(xml::XmlClassFactory);
	T_FORCE_LINK_REF(sql::SqlClassFactory);
	T_FORCE_LINK_REF(sql::Sqlite3ClassFactory);

	// Register all runtime classes, first collect all classes
	// and then register them in class dependency order.
	std::set< const TypeInfo* > runtimeClassFactoryTypes;
	type_of< IRuntimeClassFactory >().findAllOf(runtimeClassFactoryTypes, false);
	for (std::set< const TypeInfo* >::const_iterator i = runtimeClassFactoryTypes.begin(); i != runtimeClassFactoryTypes.end(); ++i)
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >((*i)->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}

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

	// Run
	Ref< AutoRuntimeClass< Run > > classRun = new AutoRuntimeClass< Run >();
	classRun->addConstructor();
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
	classRun->addMethod("loadModule", &Run_loadModule);
	registrar.registerClass(classRun);

	// Register all classes to script manager; in class hierarchy order.
	registrar.registerClassesInOrder(g_scriptManager);
}

int32_t executeRun(const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	// Compile script into a runnable blob.
	Ref< script::IScriptBlob > scriptBlob = g_scriptManager->compile(fileName.getPathName(), text, 0);
	if (!scriptBlob)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Create script context..
	Ref< script::IScriptContext > scriptContext = g_scriptManager->createContext(false);
	if (!scriptContext)
		return 1;

	// Setup globals in script context.
	Ref< Environment > environment = OS::getInstance().getEnvironment();
	environment->set(L"RUN_SCRIPT", fileName.getPathName());
	scriptContext->setGlobal("environment", Any::fromObject(environment));
	scriptContext->setGlobal("run", Any::fromObject(new Run()));
	scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	scriptContext->setGlobal("os", Any::fromObject(&OS::getInstance()));
	scriptContext->setGlobal("stdout", Any::fromObject(new StdOutput(stdout)));
	scriptContext->setGlobal("stderr", Any::fromObject(new StdOutput(stderr)));

	// Load script into context.
	scriptContext->load(scriptBlob);

	// Execute optional "main" function.
	Any retval = Any::fromInt32(0);
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

	return retval.getInt32();
}

int32_t executeTemplate(const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
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

	Ref< script::IScriptBlob > scriptBlob = g_scriptManager->compile(fileName.getPathName(), ss.str(), 0);
	if (!scriptBlob)
	{
		log::error << L"Unable to compile script" << Endl;
		return 1;
	}

	// Transform arguments into script array.
	std::vector< std::wstring > args;
	for (int32_t i = 1; i < cmdLine.getCount(); ++i)
		args.push_back(cmdLine.getString(i));

	Ref< script::IScriptContext > scriptContext = g_scriptManager->createContext(false);
	if (!scriptContext)
		return 1;

	Ref< Environment > environment = OS::getInstance().getEnvironment();
	environment->set(L"RUN_SCRIPT", fileName.getPathName());
	scriptContext->setGlobal("environment", Any::fromObject(environment));
	scriptContext->setGlobal("run", Any::fromObject(new Run()));
	scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	scriptContext->setGlobal("os", Any::fromObject(&OS::getInstance()));
	scriptContext->setGlobal("stdout", Any::fromObject(new StdOutput(stdout)));
	scriptContext->setGlobal("stderr", Any::fromObject(new StdOutput(stderr)));
	scriptContext->setGlobal("output", Any::fromObject(o));
	scriptContext->setGlobal("args", CastAny< std::vector< std::wstring > >::set(args));

	scriptContext->load(scriptBlob);

	safeDestroy(scriptContext);

	log::info << o->getProduct() << Endl;

	return 0;
}

	}
}

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Traktor.Run.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
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
		log::error << L"Failed to open \"" << fileName.getPathName() << L"\"." << Endl;
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

	g_scriptManager = new script::ScriptManagerLua();
	if (!g_scriptManager)
	{
		log::error << L"Failed to create script manager." << Endl;
		return 1;
	}

	run::registerRuntimeClasses();

	net::Network::initialize();

	int32_t result = 1;

	bool explicitRun = cmdLine.hasOption(L"as-run");
	bool explicitTemplate = cmdLine.hasOption(L"as-template");

	if ((explicitRun && !explicitTemplate) || compareIgnoreCase< std::wstring >(fileName.getExtension(), L"run") == 0)
		result = run::executeRun(text, fileName, cmdLine);
	else if ((!explicitRun && explicitTemplate) || compareIgnoreCase< std::wstring >(fileName.getExtension(), L"template") == 0)
		result = run::executeTemplate(text, fileName, cmdLine);
	else
		log::error << L"Unknown file type \"" << fileName.getExtension() << L"\"; must be either \"run\" or \"template\"." << Endl;

	net::Network::finalize();

	safeDestroy(g_scriptManager);
	return result;
}

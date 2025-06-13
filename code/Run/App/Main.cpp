/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Debug/CallStack.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/EnterLeave.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/System/Environment.h"
#include "Core/System/OS.h"
#include "Net/Network.h"
#include "Run/App/ProduceOutput.h"
#include "Run/App/Run.h"
#include "Run/App/StdOutput.h"
#include "Script/IScriptContext.h"
#include "Script/IScriptDebugger.h"
#include "Script/Lua/ScriptCompilerLua.h"
#include "Script/Lua/ScriptManagerLua.h"
#include "Script/StackFrame.h"
#include "Script/Value.h"
#include "Script/ValueObject.h"
#include "Script/Variable.h"

#if defined(_WIN32)
#	include <windows.h>
#endif

namespace
{

traktor::Ref< traktor::script::IScriptCompiler > g_scriptCompiler;
traktor::Ref< traktor::script::IScriptManager > g_scriptManager;

}

#if defined(_WIN32)

LONG WINAPI
exceptionVectoredHandler(struct _EXCEPTION_POINTERS* ep)
{
	bool ouputCallStack = true;

	switch (ep->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
	case EXCEPTION_DATATYPE_MISALIGNMENT:
	case EXCEPTION_STACK_OVERFLOW:
	case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_IN_PAGE_ERROR:
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_INVALID_DISPOSITION:
	case EXCEPTION_GUARD_PAGE:
		ouputCallStack = true;
		break;

	case EXCEPTION_BREAKPOINT:
	case EXCEPTION_SINGLE_STEP:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
	case EXCEPTION_FLT_DENORMAL_OPERAND:
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case EXCEPTION_FLT_INEXACT_RESULT:
	case EXCEPTION_FLT_INVALID_OPERATION:
	case EXCEPTION_FLT_OVERFLOW:
	case EXCEPTION_FLT_STACK_CHECK:
	case EXCEPTION_FLT_UNDERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_INT_OVERFLOW:
	default:
		ouputCallStack = false;
		break;
	}

	if (ouputCallStack)
	{
		traktor::log::info << L"RUNTIME EXCEPTION OCCURED AT:" << traktor::Endl;

		void* callstack[16] = { nullptr };
		traktor::getCallStack(16, callstack, 2);
		for (int32_t i = 0; i < 16; ++i)
		{
			std::wstring symbol;
			traktor::getSymbolFromAddress(callstack[i], symbol);

			std::wstring fileName;
			int32_t line;
			traktor::getSourceFromAddress(callstack[i], fileName, line);

			traktor::log::info << L"\t" << traktor::str(L"0x%016x", callstack[i]) << L"    " << symbol << L"    " << fileName << L"(" << line << L")" << traktor::Endl;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

bool registerFileAssociation(const std::wstring& extension, const std::wstring& progId, const std::wstring& description, const std::wstring& executablePath)
{
	HKEY hKey;
	LONG result;
	std::wstring keyPath;
	DWORD disposition;

	keyPath = L"Software\\Classes\\" + extension;
	result = RegCreateKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, &disposition);
	if (result != ERROR_SUCCESS)
		return false;

	RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast< const BYTE* >(progId.c_str()), static_cast< DWORD >((progId.size() + 1) * sizeof(wchar_t)));
	RegCloseKey(hKey);

	keyPath = L"Software\\Classes\\" + progId;
	result = RegCreateKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, &disposition);
	if (result != ERROR_SUCCESS)
		return false;

	RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast< const BYTE* >(description.c_str()), static_cast< DWORD >((description.size() + 1) * sizeof(wchar_t)));
	RegCloseKey(hKey);

	keyPath = L"Software\\Classes\\" + progId + L"\\shell\\open\\command";
	result = RegCreateKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &hKey, &disposition);
	if (result != ERROR_SUCCESS)
		return false;

	const std::wstring command = L"\"" + executablePath + L"\" \"%1\"";
	RegSetValueExW(hKey, nullptr, 0, REG_SZ, reinterpret_cast< const BYTE* >(command.c_str()), static_cast< DWORD >((command.size() + 1) * sizeof(wchar_t)));
	RegCloseKey(hKey);

	return true;
}

#endif

namespace traktor::run
{

/*! Simple script debugger. */
class ScriptDebuggerListener : public script::IScriptDebugger::IListener
{
public:
	virtual void debugeeStateChange(script::IScriptDebugger* scriptDebugger)
	{
		if (!scriptDebugger->isRunning())
		{
			for (uint32_t depth = 0;; ++depth)
			{
				Ref< script::StackFrame > sf;
				if (!scriptDebugger->captureStackFrame(depth, sf))
					break;
				T_FATAL_ASSERT(sf);
				log::info << depth << L". " << sf->getFileName() << L" " << sf->getFunctionName() << L" (" << (sf->getLine() + 1) << L")" << Endl;

				RefArray< script::Variable > locals;
				scriptDebugger->captureLocals(depth, locals);
				for (auto local : locals)
					if (const script::Value* value = dynamic_type_cast< const script::Value* >(local->getValue()))
						log::info << L"\t\"" << local->getName() << L"\" = " << value->getLiteral() << Endl;
					else if (const script::ValueObject* value = dynamic_type_cast< const script::ValueObject* >(local->getValue()))
						log::info << L"\t\"" << local->getName() << L"\" = " << value->getValueOf() << Endl;
			}
			scriptDebugger->actionContinue();
		}
	}
};

/*! Execute run script. */
int32_t executeRun(const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	// Compile script into a runnable blob.
	Ref< script::IScriptBlob > scriptBlob = g_scriptCompiler->compile(fileName.getFileName(), text, nullptr);
	if (!scriptBlob)
	{
		log::error << L"Unable to compile script \"" << fileName.getPathName() << L"\"." << Endl;
		return 1;
	}

	// Create script context..
	Ref< script::IScriptContext > scriptContext = g_scriptManager->createContext(false);
	if (!scriptContext)
		return 2;

	// Expose some environment variables of running script.
	OS::getInstance().setEnvironment(L"RUN_SCRIPT", fileName.getPathName());
	OS::getInstance().setEnvironment(L"RUN_SCRIPT_PATH", fileName.getPathOnly());

	// Setup globals in script context.
	scriptContext->setGlobal("environment", Any::fromObject(OS::getInstance().getEnvironment()));
	scriptContext->setGlobal("run", Any::fromObject(new Run(g_scriptCompiler, g_scriptManager, scriptContext, cmdLine)));
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

		const Any a = CastAny< std::vector< std::wstring > >::set(args);
		retval = scriptContext->executeFunction("main", 1, &a);
	}

	safeDestroy(scriptContext);

	return retval.getInt32();
}

/*! Execute template script. */
int32_t executeTemplate(const std::wstring& text, const Path& fileName, const CommandLine& cmdLine)
{
	Ref< ProduceOutput > o = new ProduceOutput();

	// Transform into valid script.
	StringOutputStream ss;
	size_t offset = 0;

	for (;;)
	{
		const size_t s = text.find(L"<!--", offset);
		if (s == text.npos)
			break;

		const size_t e = text.find(L"--!>", s);
		if (e == text.npos)
		{
			log::error << L"Template syntax error; missing end." << Endl;
			return 1;
		}

		const int32_t id = o->addSection(text.substr(offset, s - offset));
		ss << L"output:printSection(" << id << L")" << Endl;
		ss << text.substr(s + 5, e - s - 5) << Endl;

		offset = e + 4;
	}

	const int32_t id = o->addSection(text.substr(offset));
	ss << L"output:printSection(" << id << L")" << Endl;

	Ref< script::IScriptBlob > scriptBlob = g_scriptCompiler->compile(fileName.getFileName(), ss.str(), nullptr);
	if (!scriptBlob)
	{
		log::error << L"Unable to compile script \"" << fileName.getPathName() << L"\"." << Endl;
		return 2;
	}

	// Transform arguments into script array.
	std::vector< std::wstring > args;
	for (int32_t i = 1; i < cmdLine.getCount(); ++i)
		args.push_back(cmdLine.getString(i));

	Ref< script::IScriptContext > scriptContext = g_scriptManager->createContext(false);
	if (!scriptContext)
		return 3;

	// Expose some environment variables of running script.
	OS::getInstance().setEnvironment(L"RUN_SCRIPT", fileName.getPathName());
	OS::getInstance().setEnvironment(L"RUN_SCRIPT_PATH", fileName.getPathOnly());

	// Setup globals in script context.
	scriptContext->setGlobal("environment", Any::fromObject(OS::getInstance().getEnvironment()));
	scriptContext->setGlobal("run", Any::fromObject(new Run(g_scriptCompiler, g_scriptManager, scriptContext, cmdLine)));
	scriptContext->setGlobal("fileSystem", Any::fromObject(&FileSystem::getInstance()));
	scriptContext->setGlobal("os", Any::fromObject(&OS::getInstance()));
	scriptContext->setGlobal("stdout", Any::fromObject(new StdOutput(stdout)));
	scriptContext->setGlobal("stderr", Any::fromObject(new StdOutput(stderr)));
	scriptContext->setGlobal("output", Any::fromObject(o));
	scriptContext->setGlobal("args", CastAny< std::vector< std::wstring > >::set(args));

	if (!scriptContext->load(scriptBlob))
		return 4;

	safeDestroy(scriptContext);

	if (cmdLine.hasOption('o', L"output"))
	{
		const std::wstring fileName = cmdLine.getOption('o', L"output").getString();
		Ref< IStream > fs = FileSystem::getInstance().open(fileName, File::FmWrite);
		if (fs)
		{
			FileOutputStream(fs, new Utf8Encoding()) << o->getProduct() << Endl;
			fs->close();
		}
		else
		{
			log::error << L"Unable to create output file \"" << fileName << L"\"." << Endl;
			return 5;
		}
	}
	else
		log::info << o->getProduct() << Endl;

	return 0;
}

}

using namespace traktor;

int main(int argc, const char** argv)
{
	int32_t result = 1;
	try
	{
#if defined(_WIN32) && !defined(_DEBUG)
		PVOID eh = nullptr;
		T_ANONYMOUS_VAR(EnterLeave)(
			[&]() {
			eh = AddVectoredExceptionHandler(1, exceptionVectoredHandler);
			},
			[&]() {
			RemoveVectoredExceptionHandler(eh);
		});
		SetErrorMode(SEM_NOGPFAULTERRORBOX);
#endif

		CommandLine cmdLine(argc, argv);

		if (
			cmdLine.getCount() < 1
#if defined(_WIN32)
			&& !cmdLine.hasOption(L"register-extensions")
#endif
		)
		{
			log::info << L"Traktor.Run.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
			log::info << L"Visit https://github.com/apistol78/traktor for more information." << Endl;
			log::info << Endl;
			log::info << L"Usage: Run (option(s)) [<file>.run|<file>.template] (args ...)" << Endl;
			log::info << Endl;
			log::info << L"  Options:" << Endl;
			log::info << L"    --as-run                   Run file as run" << Endl;
			log::info << L"    --as-template              Run file as template" << Endl;
			log::info << L"    --debug                    Attach debugger" << Endl;
#if defined(_WIN32)
			log::info << L"    --register-extensions      Register .run and .template extensions" << Endl;
#endif
			log::info << Endl;
			log::info << L"  For .run files:" << Endl;
			log::info << L"    -e,--entry-point=function  Script entry point (default \"main\")" << Endl;
			log::info << Endl;
			log::info << L"  For .template files:" << Endl;
			log::info << L"    -o,--output=filename       Output file (default stdout)" << Endl;
			return 0;
		}

#if defined(_WIN32)
		if (cmdLine.hasOption(L"register-extensions"))
		{
			const std::wstring executable = OS::getInstance().getExecutable().getPathNameOS();
			if (!registerFileAssociation(L".run", L"TraktorRunScript", L"Traktor Run Script", executable))
			{
				log::error << L"Failed to register file extension .run" << Endl;
				return 1;
			}
			if (!registerFileAssociation(L".template", L"TraktorRunTemplate", L"Traktor Run Script", executable))
			{
				log::error << L"Failed to register file extension .template" << Endl;
				return 1;
			}

			log::info << L"File extension .run and .template association registered successfully." << Endl;
			return 0;
		}
#endif

		// Check if environment is already set, else set to current working directory.
		std::wstring home;
		if (!OS::getInstance().getEnvironment(L"TRAKTOR_HOME", home))
		{
			const Path originalCwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
			const Path executablePath = OS::getInstance().getExecutable().getPathOnly();
			FileSystem::getInstance().setCurrentVolumeAndDirectory(executablePath);

			while (!FileSystem::getInstance().exist(L"LICENSE.txt"))
			{
				const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
				const Path pwd = cwd.getPathOnly();
				if (cwd == pwd)
					break;
				FileSystem::getInstance().setCurrentVolumeAndDirectory(pwd);
			}

			const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
			if (!cwd.empty())
			{
				OS::getInstance().setEnvironment(L"TRAKTOR_HOME", cwd.getPathNameOS());
				FileSystem::getInstance().setCurrentVolumeAndDirectory(originalCwd);
			}
		}

		// Read script into memory.
		const Path fileName = cmdLine.getString(0);

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
#if defined(__LINUX__)
		// Strip shebang line on linux.
		if (reader.readLine(tmp) >= 0)
		{
			if (!tmp.starts_with(L"#!"))
				ss << tmp << Endl;
		}
#endif
		while (reader.readLine(tmp) >= 0)
			ss << tmp << Endl;

		safeClose(file);

		const std::wstring text = ss.str();

		// Create script manager.
		g_scriptCompiler = new script::ScriptCompilerLua();
		g_scriptManager = new script::ScriptManagerLua();

		run::Run::registerRuntimeClasses(g_scriptManager);
		net::Network::initialize();

		// Execute script.
		const bool explicitRun = cmdLine.hasOption(L"as-run");
		const bool explicitTemplate = cmdLine.hasOption(L"as-template");

		if (cmdLine.hasOption(L"debug"))
		{
			Ref< script::IScriptDebugger > scriptDebugger = g_scriptManager->createDebugger();
			scriptDebugger->addListener(new run::ScriptDebuggerListener());
		}

		if ((explicitRun && !explicitTemplate) || compareIgnoreCase(fileName.getExtension(), L"run") == 0)
			result = run::executeRun(text, fileName, cmdLine);
		else if ((!explicitRun && explicitTemplate) || compareIgnoreCase(fileName.getExtension(), L"template") == 0)
			result = run::executeTemplate(text, fileName, cmdLine);
		else
			log::error << L"Unknown file type \"" << fileName.getExtension() << L"\"; must be either \"run\" or \"template\"." << Endl;

		g_scriptCompiler = nullptr;

		net::Network::finalize();
		safeDestroy(g_scriptManager);
	}
	catch (...)
	{
		log::error << L"Unhandled exception occurred." << Endl;
	}
	return result;
}

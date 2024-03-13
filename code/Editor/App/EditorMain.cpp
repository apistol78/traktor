/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(_WIN32)
#	include <Windows.h>
#endif
#include "Core/Debug/Debugger.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Log/LogRedirectTarget.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/Splash.h"
#include "Net/Network.h"
#include "Ui/Application.h"

#if defined(_WIN32)
#	include "Ui/Win32/WidgetFactoryWin32.h"
typedef traktor::ui::WidgetFactoryWin32 WidgetFactoryImpl;
#elif defined(__APPLE__)
#	include "Ui/Cocoa/WidgetFactoryCocoa.h"
typedef traktor::ui::WidgetFactoryCocoa WidgetFactoryImpl;
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Ui/X11/WidgetFactoryX11.h"
typedef traktor::ui::WidgetFactoryX11 WidgetFactoryImpl;
#endif

#if defined(__LINUX__) || defined(__RPI__)
#	include <X11/Xlib.h>
#endif

using namespace traktor;

#if defined(_WIN32)
// NVidia hack to get Optimus to enable NVidia GPU when possible.
extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

namespace
{

class LogStreamTarget : public ILogTarget
{
public:
	explicit LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		(*m_stream) << L"[" << DateTime::now().format(L"%H:%M:%S") << L"] " << str << Endl;
	}

private:
	Ref< OutputStream > m_stream;
};

}

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	wchar_t file[MAX_PATH] = L"";
	GetModuleFileName(NULL, file, sizeof_array(file));
	CommandLine cmdLine(file, mbstows(szCmdLine));
#endif

#if defined(__LINUX__) || defined(__RPI__)
	// Initialize X11 thread primitives; thus must be performed very early.
	XInitThreads();
#endif

	// Ensure temporary folder exist.
	const std::wstring writableFolder = OS::getInstance().getWritableFolderPath() + L"/Traktor/Editor/Logs";
	FileSystem::getInstance().makeAllDirectories(writableFolder);

	// Save log file.
	Ref< traktor::IStream > logFile;
#if !defined(_DEBUG)
	if (!Debugger::getInstance().isDebuggerAttached())
	{
		RefArray< File > logs = FileSystem::getInstance().find(writableFolder + L"/Editor_*.log");

		// Get "alive" log ids.
		std::vector< int32_t > logIds;
		for (auto log : logs)
		{
			const std::wstring logName = log->getPath().getFileNameNoExtension();
			const size_t p = logName.find(L'_');
			if (p != logName.npos)
			{
				const int32_t id = parseString< int32_t >(logName.substr(p + 1), -1);
				if (id != -1)
					logIds.push_back(id);
			}
		}

		int32_t nextLogId = 0;
		if (!logIds.empty())
		{
			std::sort(logIds.begin(), logIds.end());

			// Don't keep more than 10 log files.
			while (logIds.size() >= 10)
			{
				StringOutputStream ss;
				ss << L"Editor_" << logIds.front() << L".log";
				FileSystem::getInstance().remove(ss.str());
				logIds.erase(logIds.begin());
			}

			nextLogId = logIds.back() + 1;
		}

		// Create new log file.
		StringOutputStream ss;
		ss << writableFolder << L"/Editor_" << nextLogId << L".log";
		logFile = FileSystem::getInstance().open(ss.str(), File::FmWrite);
		if (logFile)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logTarget = new LogStreamTarget(logStream);

			Ref< ILogTarget > defaultInfoLog = log::info.getGlobalTarget();
			Ref< ILogTarget > defaultWarningLog = log::info.getGlobalTarget();
			Ref< ILogTarget > defaultErrorLog = log::info.getGlobalTarget();

			log::info.setGlobalTarget(new LogRedirectTarget(defaultInfoLog, logTarget));
			log::warning.setGlobalTarget(new LogRedirectTarget(defaultWarningLog, logTarget));
			log::error.setGlobalTarget(new LogRedirectTarget(defaultErrorLog, logTarget));

			log::info << L"Log file \"" << ss.str() << L"\" created." << Endl;
		}
		else
			log::error << L"Unable to create log file; logging only to std pipes." << Endl;
	}
#endif

	// Check if environment is already set, else set to current working directory.
	std::wstring home;
	if (!OS::getInstance().getEnvironment(L"TRAKTOR_HOME", home))
	{
		while (!FileSystem::getInstance().exist(L"LICENSE.txt"))
		{
			const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
			const Path pwd = cwd.getPathOnly();
			if (cwd == pwd)
			{
				log::error << L"No LICENSE.txt file found." << Endl;
				return 1;
			}
			FileSystem::getInstance().setCurrentVolumeAndDirectory(pwd);
		}

		const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
		OS::getInstance().setEnvironment(L"TRAKTOR_HOME", cwd.getPathNameOS());
	}

	ui::Application::getInstance()->initialize(
		new WidgetFactoryImpl(),
		nullptr
	);

	net::Network::initialize();

	try
	{
#if !defined(_DEBUG)
		Ref< editor::Splash > splash;
		if (!cmdLine.hasOption(L"no-splash"))
		{
			splash = new editor::Splash();
			splash->create();

			for (int32_t i = 0; i < 10; ++i)
				ui::Application::getInstance()->process();
		}
#endif

		Ref< editor::EditorForm > editorForm = new editor::EditorForm();
		if (editorForm->create(cmdLine))
		{
#if !defined(_DEBUG)
			if (splash)
			{
				splash->hide();
				safeDestroy(splash);
			}
#endif

			ui::Application::getInstance()->execute();
			safeDestroy(editorForm);
		}
	}
	catch (...)
	{
		traktor::log::error << L"Unhandled exception, application terminated." << Endl;
	}

	net::Network::finalize();

	ui::Application::getInstance()->finalize();

#if !defined(_DEBUG)
	safeClose(logFile);
#endif

#if 0
	Object::setReferenceDebugger(nullptr);
#endif
	return 0;
}

#if defined(_DEBUG)
#	include "Core/CycleRefDebugger.h"
#endif
#include "Core/Debug/Debugger.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/Split.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Editor/App/EditorForm.h"
#include "Editor/App/Splash.h"
#include "Net/Network.h"
#include "Ui/Application.h"

#if defined(_WIN32)
#	include <Ui/Win32/EventLoopWin32.h>
#	include <Ui/Win32/WidgetFactoryWin32.h>
typedef traktor::ui::EventLoopWin32 EventLoopImpl;
typedef traktor::ui::WidgetFactoryWin32 WidgetFactoryImpl;
#elif defined(__APPLE__)
#	include <Ui/Cocoa/EventLoopCocoa.h>
#	include <Ui/Cocoa/WidgetFactoryCocoa.h>
typedef traktor::ui::EventLoopCocoa EventLoopImpl;
typedef traktor::ui::WidgetFactoryCocoa WidgetFactoryImpl;
/*
#elif defined(__GNUC__)
#	include <Ui/Gtk/EventLoopGtk.h>
#	include <Ui/Gtk/WidgetFactoryGtk.h>
typedef traktor::ui::EventLoopGtk EventLoopImpl;
typedef traktor::ui::WidgetFactoryGtk WidgetFactoryImpl;
*/
#else
#	include <Ui/Wx/EventLoopWx.h>
#	include <Ui/Wx/WidgetFactoryWx.h>
typedef traktor::ui::EventLoopWx EventLoopImpl;
typedef traktor::ui::WidgetFactoryWx WidgetFactoryImpl;
#endif

using namespace traktor;

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

#if 0
	CycleRefDebugger cycleDebugger;
	Object::setReferenceDebugger(&cycleDebugger);
#endif

#if defined(__APPLE__)
	// Everything should live inside the bundle and not have
	// a special location in the file system; thus we override
	// our special environment variable to point to our bundle.
	if (!Debugger::getInstance().isDebuggerAttached())
	{
		std::wstring bundlePath;
		if (OS::getInstance().getEnvironment(L"BUNDLE_PATH", bundlePath))
			setenv("TRAKTOR_HOME", wstombs(bundlePath + L"/Contents/Resources").c_str(), 1);
	}

	// Log some relevant environment variables to ease debugging.
	std::wstring check;

	OS::getInstance().getEnvironment(L"TRAKTOR_HOME", check);
	log::info << L"TRAKTOR_HOME = \"" << check << L"\"" << Endl;

	OS::getInstance().getEnvironment(L"DYLD_LIBRARY_PATH", check);
	log::info << L"DYLD_LIBRARY_PATH = \"" << check << L"\"" << Endl;
#endif

	ui::Application::getInstance()->initialize(
		new EventLoopImpl(),
		new WidgetFactoryImpl()
	);

	net::Network::initialize();

	try
	{
#if !defined(_DEBUG)
		Ref< editor::Splash > splash = new editor::Splash();
		splash->create();

		for (int i = 0; i < 10; ++i)
			ui::Application::getInstance()->process();
#endif

		Ref< editor::EditorForm > editorForm = new editor::EditorForm();
		if (editorForm->create(cmdLine))
		{
#if !defined(_DEBUG)
			splash->hide();
#endif

			ui::Application::getInstance()->execute();
			editorForm->destroy();
		}

#if !defined(_DEBUG)
		splash->destroy();
#endif
	}
	catch (...)
	{
		traktor::log::error << L"Unhandled exception, application terminated" << Endl;
	}

	net::Network::finalize();

	ui::Application::getInstance()->finalize();

#if 0
	Object::setReferenceDebugger(0);
#endif
	return 0;
}

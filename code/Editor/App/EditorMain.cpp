#if defined(_WIN32)
#	include <windows.h>
#endif
#include "Ui/Application.h"
#include "Editor/App/Splash.h"
#include "Editor/App/EditorForm.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/Split.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Log/Log.h"

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
#elif defined(__GNUC__)
#	include <Ui/Gtk/EventLoopGtk.h>
#	include <Ui/Gtk/WidgetFactoryGtk.h>
typedef traktor::ui::EventLoopGtk EventLoopImpl;
typedef traktor::ui::WidgetFactoryGtk WidgetFactoryImpl;
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
	GetModuleFileName(NULL, file, sizeof(file));

	CommandLine cmdLine(file, mbstows(szCmdLine));
#endif

	ui::Application::getInstance().initialize(
		new EventLoopImpl(),
		new WidgetFactoryImpl()
	);

	try
	{
#if !defined(_DEBUG)
		editor::Splash splash;
		splash.create();

		for (int i = 0; i < 10; ++i)
		{
			ui::Application::getInstance().process();
			ThreadManager::getInstance().getCurrentThread()->sleep(50);
		}
#endif

		editor::EditorForm editorForm;
		if (editorForm.create(cmdLine))
		{
#if !defined(_DEBUG)
			splash.hide();
#endif

			ui::Application::getInstance().execute();
			editorForm.destroy();
		}

#if !defined(_DEBUG)
		splash.destroy();
#endif
	}
	catch (...)
	{
		traktor::log::error << L"Unhandled exception, application terminated" << Endl;
	}

	ui::Application::getInstance().finalize();
	return 0;
}

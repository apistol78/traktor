#if defined(_WIN32)
#include <windows.h>
#endif
#include "Ui/Application.h"
#include "Editor/App/Splash.h"
#include "Editor/App/EditorForm.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SplitString.h"
#include "Core/Heap/Heap.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
#include "Core/Log/Log.h"

#include "Core/Timer/Profiler.h"
#if defined(T_PROFILER_ENABLE)
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#endif

#define T_UI_IMPL_WX	1
#define T_UI_IMPL_WIN32	2

#if !defined(T_UI_IMPL)
#if defined(_WIN32)
#define T_UI_IMPL T_UI_IMPL_WIN32
#else
#define T_UI_IMPL T_UI_IMPL_WX
#endif
#endif

#if T_UI_IMPL == T_UI_IMPL_WX
#include "Ui/Wx/EventLoopWx.h"
#include "Ui/Wx/WidgetFactoryWx.h"
#elif T_UI_IMPL == T_UI_IMPL_WIN32
#include "Ui/Win32/EventLoopWin32.h"
#include "Ui/Win32/WidgetFactoryWin32.h"
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

#if T_UI_IMPL == T_UI_IMPL_WX
	ui::Application::getInstance().initialize(
		new ui::EventLoopWx(),
		new ui::WidgetFactoryWx()
	);
#elif T_UI_IMPL == T_UI_IMPL_WIN32
	ui::Application::getInstance().initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32()
	);
#endif

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
	
#if defined(T_PROFILER_ENABLE)
	Ref< Stream> stream = FileSystem::getInstance().open(L"Profile.xml", File::FmWrite);
	if (stream)
	{
		FileOutputStream adapter(stream, gc_new< Utf8Encoding >());
		Profiler::getInstance().report(adapter);
		stream->close();
	}
	else
	{
		traktor::log::warning << L"Unable to save profile report" << Endl;
		Profiler::getInstance().report(traktor::log::info);
	}
#endif

	Heap::getInstance().collectAll();
	return 0;
}

#if defined(_WIN32)
#	include <windows.h>
#endif
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/Split.h"
#include "Core/Thread/Mutex.h"
#include "Drone/App/DroneForm.h"
#include "Ui/Application.h"
#if defined(_WIN32)
#	include "Ui/Win32/EventLoopWin32.h"
#	include "Ui/Win32/WidgetFactoryWin32.h"
#else
#	include "Ui/Wx/EventLoopWx.h"
#	include "Ui/Wx/WidgetFactoryWx.h"
#endif

using namespace traktor;

#if defined(_WIN32)
Mutex g_globalMutex(Guid(L"{820A4A35-272C-4C7A-9477-32D0D59AFE69}"));
#endif

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	// If global mutex already was created then
	// the server is already running.
	if (g_globalMutex.existing())
		return 0;

	wchar_t file[MAX_PATH] = L"";
	GetModuleFileName(NULL, file, sizeof_array(file));
	CommandLine cmdLine(file, mbstows(szCmdLine));
#endif

#if defined(_WIN32)
	ui::Application::getInstance()->initialize(
		new ui::WidgetFactoryWin32(),
		nullptr
	);
#else
	ui::Application::getInstance()->initialize(
		new ui::WidgetFactoryWx(),
		nullptr
	);
#endif

	drone::DroneForm form;
	if (form.create(cmdLine))
	{
		ui::Application::getInstance()->execute();
		form.destroy();
	}

	ui::Application::getInstance()->finalize();
	return 0;
}

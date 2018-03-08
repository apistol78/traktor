/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	std::vector< std::wstring > argv;

	// If global mutex already was created then
	// the server is already running.
	if (g_globalMutex.existing())
		return 0;

	TCHAR szFilename[MAX_PATH] = _T("");
	GetModuleFileName(NULL, szFilename, sizeof(szFilename));
	argv.push_back(tstows(szFilename));

	Split< std::wstring >::any(mbstows(szCmdLine), L" \t", argv);
	CommandLine cmdLine(argv);
#endif

#if defined(_WIN32)
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32(),
		0
	);
#else
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWx(),
		new ui::WidgetFactoryWx(),
		0
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

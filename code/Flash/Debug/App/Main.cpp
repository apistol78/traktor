/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#endif
#include <Core/Io/FileSystem.h>
#include <Core/Misc/CommandLine.h>
#include <Net/Network.h>
#include <Ui/Application.h>
#include <Ui/StyleSheet.h>
#if defined(_WIN32)
#	include <Ui/Win32/EventLoopWin32.h>
#	include <Ui/Win32/WidgetFactoryWin32.h>
#elif defined(__APPLE__)
#	include <Ui/Cocoa/EventLoopCocoa.h>
#	include <Ui/Cocoa/WidgetFactoryCocoa.h>
#else
#	include <Ui/Wx/EventLoopWx.h>
#	include <Ui/Wx/WidgetFactoryWx.h>
#endif
#include <Xml/XmlDeserializer.h>
#include "MainForm.h"

using namespace traktor;

#if defined(_WIN32)
// NVidia hack to get Optimus to enable NVidia GPU when possible.
extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

Ref< ui::StyleSheet > loadStyleSheet(const Path& pathName)
{
	Ref< traktor::IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
	if (file)
		return xml::XmlDeserializer(file).readObject< ui::StyleSheet >();
	else
		return 0;
}

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

	net::Network::initialize();

#if defined(_WIN32)
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32(),
		0
	);
#elif defined(__APPLE__)
	ui::Application::getInstance()->initialize(
		new ui::EventLoopCocoa(),
		new ui::WidgetFactoryCocoa(),
		0
	);
#else
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWx(),
		new ui::WidgetFactoryWx(),
		0
	);
#endif

	Ref< ui::StyleSheet > styleSheet = loadStyleSheet(L"$(TRAKTOR_HOME)/res/themes/Dark/StyleSheet.xss");
	if (!styleSheet)
	{
		log::error << L"Unable to load stylesheet." << Endl;
		return false;
	}
	ui::Application::getInstance()->setStyleSheet(styleSheet);

	flash::MainForm form;
	if (form.create())
	{
		ui::Application::getInstance()->execute();
		form.destroy();
	}

	ui::Application::getInstance()->finalize();

	net::Network::finalize();

	return 0;
}

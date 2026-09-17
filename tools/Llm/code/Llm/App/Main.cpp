/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(_WIN32)
#	include <windows.h>
#endif
#include "Core/Config.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Llm/App/ChatForm.h"
#include "Llm/App/ChatView.h"
#include "Llm/Context.h"
#include "Llm/SelfTest.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#if defined(_WIN32)
#	include "Ui/Win32/WidgetFactoryWin32.h"
#elif defined(__APPLE__)
#	include "Ui/Cocoa/WidgetFactoryCocoa.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Ui/Wl/WidgetFactoryWl.h"
#	include "Ui/X11/WidgetFactoryX11.h"
#endif

using namespace traktor;

namespace
{

#if defined(__LINUX__) || defined(__RPI__)

/*! True when the session is Wayland rather than X11. */
bool isWayland()
{
	std::wstring value;
	if (OS::getInstance().getEnvironment(L"WAYLAND_DISPLAY", value) && !value.empty())
		return true;

	if (OS::getInstance().getEnvironment(L"XDG_SESSION_TYPE", value))
		return value == L"wayland";

	return false;
}

#endif

ui::IWidgetFactory* createWidgetFactory()
{
#if defined(_WIN32)
	return new ui::WidgetFactoryWin32();
#elif defined(__APPLE__)
	return new ui::WidgetFactoryCocoa();
#elif defined(__LINUX__) || defined(__RPI__)
	if (isWayland())
		return new ui::WidgetFactoryWl();
	else
		return new ui::WidgetFactoryX11();
#else
	return nullptr;
#endif
}

}

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int argc, const char** argv)
{
	const CommandLine commandLine(argc, argv);
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	wchar_t file[MAX_PATH] = L"";
	GetModuleFileName(NULL, file, sizeof(file));

	const CommandLine commandLine(file, mbstows(szCmdLine));
#endif

	// Bring up check; loads a model, reports it and runs a short greedy
	// generation without opening a window.
	if (commandLine.hasOption(L"selftest"))
	{
		if (commandLine.getCount() < 1)
		{
			log::error << L"Usage: Llm.App -selftest [-tokens=N] [-context=N] <model.gguf>" << Endl;
			return 1;
		}

		const int32_t tokenCount = commandLine.hasOption(L"tokens") ? commandLine.getOption(L"tokens").getInteger() : 16;

		// Same default as the window. A bring-up report generates a handful of
		// tokens, so sizing the cache for the model's full trained context
		// would cost gigabytes to prove nothing.
		const int32_t contextLength = commandLine.hasOption(L"context") ? commandLine.getOption(L"context").getInteger() : llm::c_defaultContextLength;

		return llm::runSelfTest(Path(commandLine.getString(0)), tokenCount, contextLength);
	}

	if (!ui::Application::getInstance()->initialize(createWidgetFactory(), nullptr))
	{
		log::error << L"Unable to initialize user interface." << Endl;
		return 1;
	}

	// The default sheet has no entries for the transcript; merge ours over it
	// rather than replacing it, so every other widget keeps its own styling.
	ui::Application::getInstance()->appendStyleSheet(llm::ChatView::createStyleSheet());

	int32_t result = 0;
	{
		llm::ChatForm form;
		if (form.create(commandLine))
		{
			ui::Application::getInstance()->execute();
			form.destroy();
		}
		else
		{
			log::error << L"Unable to create main window." << Endl;
			result = 1;
		}
	}

	ui::Application::getInstance()->finalize();
	return result;
}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "SolutionForm.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "Ui/Application.h"
#if defined(_WIN32)
#	include "Ui/Win32/WidgetFactoryWin32.h"
#elif defined(__APPLE__)
#	include "Ui/Cocoa/WidgetFactoryCocoa.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Ui/X11/WidgetFactoryX11.h"
#endif

using namespace traktor;
using namespace sb;

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
	T_FORCE_LINK_REF(ProjectDependency)
	T_FORCE_LINK_REF(ExternalDependency)

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

#if defined(_WIN32)
	ui::Application::getInstance()->initialize(
		new ui::WidgetFactoryWin32(),
		nullptr
	);
#elif defined(__APPLE__)
	ui::Application::getInstance()->initialize(
		new ui::WidgetFactoryCocoa(),
		nullptr
	);
#elif defined(__LINUX__) || defined(__RPI__)
	ui::Application::getInstance()->initialize(
		new ui::WidgetFactoryX11(),
		nullptr
	);
#endif

	SolutionForm form;
	if (form.create(cmdLine))
	{
		ui::Application::getInstance()->execute();
		form.destroy();
	}

	ui::Application::getInstance()->finalize();
	return 0;
}

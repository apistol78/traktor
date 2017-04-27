/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <emscripten.h>
#include "Amalgam/Game/IOnlineServer.h"
#include "Amalgam/Game/Impl/Application.h"
#include "Amalgam/Game/Impl/Environment.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/System/OS.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

extern "C" void __traktor__emscripten__linkage__(); 

namespace
{

Ref< amalgam::Application > g_application;

Ref< PropertyGroup > loadSettings(const Path& settingsFile)
{
	Ref< PropertyGroup > settings;

	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	return settings;
}

}

void mainLoop()
{
	if (g_application)
	{
		if (!g_application->update())
			safeDestroy(g_application);
	}
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	SystemApplication sysapp;

	__traktor__emscripten__linkage__();

	Path settingsPath = L"Application.config";
	if (cmdLine.getCount() >= 1)
		settingsPath = cmdLine.getString(0);

	traktor::log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L"); please reinstall application" << Endl;
		return 0;
	}

	traktor::log::info << L"Settings loaded successfully." << Endl;

	Path workingDirectory = FileSystem::getInstance().getAbsolutePath(settingsPath).getPathOnly();
	FileSystem::getInstance().setCurrentVolumeAndDirectory(workingDirectory);

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	traktor::log::info << L"Creating application..." << Endl;

	g_application = new amalgam::Application();
	if (!g_application->create(
		defaultSettings,
		settings,
		sysapp,
		0
	))
		return 0;

	traktor::log::info << L"Application created successfully." << Endl;

	// Ok, everything seems to be setup fine, give main loop back to Emscripten.
	emscripten_set_main_loop(&mainLoop, 0, 0);
	return 0;
}

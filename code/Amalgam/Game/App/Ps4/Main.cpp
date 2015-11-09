#include "Amalgam/Game/Impl/Application.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Serialization/DeepClone.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

bool g_going = true;

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

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	Path settingsPath = L"Application.config";
	if (cmdLine.getCount() >= 1)
		settingsPath = cmdLine.getString(0);

#if defined(T_MUTE_LOGS)
	traktor::log::debug.setBuffer(0);
	traktor::log::info.setBuffer(0);
	traktor::log::warning.setBuffer(0);
	traktor::log::error.setBuffer(0);
#endif

	traktor::log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L"); please reinstall application" << Endl;
		return 0;
	}

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	Ref< amalgam::Application > application = new amalgam::Application();
	if (application->create(
		defaultSettings,
		settings,
		0,
		0
	))
	{
		// Enter main loop.
		while (g_going)
		{
			if (!application->update())
				break;
		}

		log::debug << L"Destroying application..." << Endl;
		application->destroy();
		application = 0;
	}

	traktor::log::info << L"Bye" << Endl;
	return 0;
}

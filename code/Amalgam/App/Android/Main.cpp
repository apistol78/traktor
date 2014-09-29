#include <jni.h>
#include <android_native_app_glue.h>
#include "Amalgam/IOnlineServer.h"
#include "Amalgam/Impl/Application.h"
#include "Amalgam/Impl/Environment.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/System/OS.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

using namespace traktor;

namespace
{

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

bool saveSettings(const PropertyGroup* settings, const Path& settingsFile)
{
	T_ASSERT (settings);

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmWrite);
	if (!file)
		return false;

	bool result = xml::XmlSerializer(file).writeObject(settings);
	file->close();

	return result;
}

}

void android_main(struct android_app* state)
{
	// Make sure glue isn't stripped.
	app_dummy();

	Path settingsPath = L"ASSETS:Application.config";
	traktor::log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L"); please reinstall application" << Endl;
		traktor::log::error << L"Please reinstall application." << Endl;
		return;
	}

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	if (!settings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		traktor::log::error << L"Please reinstall application." << Endl;
		return;
	}

	Ref< amalgam::Application > application = new amalgam::Application();
	if (application->create(
		defaultSettings,
		settings,
		0,
		0
	))
	{
		struct android_poll_source* source;
		int ident;
		int events;

		while ((ident = ALooper_pollAll(true, NULL, &events, (void**)&source)) >= 0)
		{
			if (!application->update())
			{
				traktor::log::info << L"Update returned false; terminating application..." << Endl;
				break;
			}
		}

		safeDestroy(application);
	}
	else
		traktor::log::error << L"Unable to create application" << Endl;

	traktor::log::info << L"Bye" << Endl;
}

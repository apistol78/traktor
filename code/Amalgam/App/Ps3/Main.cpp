#include <sysutil/sysutil_common.h>
#include "Amalgam/IOnlineServer.h"
#include "Amalgam/Impl/Application.h"
#include "Amalgam/Impl/Environment.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/System/OS.h"
#include "Core/System/Ps3/SystemCallback.h"
#include "Online/ISaveData.h"
#include "Online/ISessionManager.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

bool g_going = true;

void callbackExit(uint64_t status, uint64_t param)
{
	if (status == CELL_SYSUTIL_REQUEST_EXITGAME)
	{
		traktor::log::debug << L"Received CELL_SYSUTIL_REQUEST_EXITGAME; terminating application" << Endl;
		g_going = false;
	}
}

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

Ref< PropertyGroup > loadSettings(amalgam::IEnvironment* environment, const std::wstring& saveDataId)
{
	amalgam::IOnlineServer* onlineServer = environment->getOnline();
	T_ASSERT (onlineServer);

	online::ISaveData* saveData = onlineServer->getSessionManager()->getSaveData();
	T_ASSERT (saveData);

	Ref< online::AttachmentResult > result = saveData->get(saveDataId);
	if (result->succeeded())
	{
		Ref< PropertyGroup > userSettingsGroup = dynamic_type_cast< PropertyGroup* >(result->get());
		if (userSettingsGroup)
			return userSettingsGroup;
		else
			traktor::log::error << L"Failed to load settings; incorrect type of attachment" << Endl;
	}
	else
		traktor::log::error << L"Failed to load settings; unable to read save data" << Endl;

	return 0;
}

void saveSettings(amalgam::IEnvironment* environment, const PropertyGroup* settings, const std::wstring& saveDataId, const online::SaveDataDesc& saveDataDesc, bool block)
{
	amalgam::IOnlineServer* onlineServer = environment->getOnline();
	T_ASSERT (onlineServer);

	online::ISaveData* saveData = onlineServer->getSessionManager()->getSaveData();
	T_ASSERT (saveData);

	if (onlineServer && onlineServer->getSessionManager())
	{
		online::ISaveData* saveData = onlineServer->getSessionManager()->getSaveData();
		if (saveData)
		{
			Ref< online::Result > result = saveData->set(saveDataId, saveDataDesc, settings, true);
			if (block)
			{
				if (!result || !result->succeeded())
					traktor::log::error << L"Failed to save settings; unable to create save data" << Endl;
			}
			else
			{
				if (!result)
					traktor::log::error << L"Failed to save settings; unable to create save data" << Endl;
			}
		}
		else
			traktor::log::error << L"Failed to save settings; no save data implementation" << Endl;
	}
	else
		traktor::log::error << L"Failed to save settings; no online implementation" << Endl;
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	SystemCallback::getInstance().add(callbackExit);

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

	std::wstring settingsSaveDataId = defaultSettings->getProperty< PropertyString >(L"Amalgam.SettingsSaveDataId");

	online::SaveDataDesc settingsSaveDataDesc;
	settingsSaveDataDesc.title = defaultSettings->getProperty< PropertyString >(L"Amalgam.SettingsSaveDataTitle");
	settingsSaveDataDesc.description = defaultSettings->getProperty< PropertyString >(L"Amalgam.SettingsSaveDataDesc");

	if (settingsSaveDataId.empty())
		traktor::log::warning << L"No settings save data id found; user settings not loaded/saved" << Endl;

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
		amalgam::Environment* environment = checked_type_cast< amalgam::Environment* >(application->getEnvironment());
		T_ASSERT (environment);

		amalgam::IOnlineServer* onlineServer = environment->getOnline();

		// Load user settings from save-data; need to reconfigure environment if settings exist.
		if (!settingsSaveDataId.empty())
		{
			Ref< PropertyGroup > userSettings = loadSettings(environment, settingsSaveDataId);
			if (userSettings)
			{
				settings = settings->mergeJoin(userSettings);
				environment->executeReconfigure();
			}
		}

		// Enter main loop.
		while (g_going)
		{
			if (!application->update())
				break;

			// Check if configuration has changed; in such case begin saving new configuration.
			if (environment->shouldReconfigure())
			{
				log::debug << L"Configuration changed; saving settings..." << Endl;
				saveSettings(environment, settings, settingsSaveDataId, settingsSaveDataDesc, false);
			}

			SystemCallback::getInstance().update();
		}

		// Save user settings as a save-data.
		saveSettings(environment, settings, settingsSaveDataId, settingsSaveDataDesc, true);

		log::debug << L"Destroying application..." << Endl;
		application->destroy();
		application = 0;
	}

	traktor::log::info << L"Bye" << Endl;
	return 0;
}

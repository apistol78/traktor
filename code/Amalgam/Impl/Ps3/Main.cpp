#include <sysutil/sysutil_common.h>
#include "Amalgam/IOnlineServer.h"
#include "Amalgam/Launch.h"
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
#include "Core/Settings/Settings.h"
#include "Core/System/OS.h"
#include "Core/System/Ps3/SystemCallback.h"
#include "Online/ISaveData.h"
#include "Online/ISessionManager.h"
#include "Xml/XmlDeserializer.h"

namespace traktor
{
	namespace amalgam
	{
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

Ref< Settings > loadSettings(const Path& settingsFile)
{
	Ref< Settings > settings;

	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}

	return settings;
}

Ref< Settings > loadSettings(IEnvironment* environment, const std::wstring& saveDataId)
{
	IOnlineServer* onlineServer = environment->getOnline();
	T_ASSERT (onlineServer);

	online::ISaveData* saveData = onlineServer->getSessionManager()->getSaveData();
	T_ASSERT (saveData);

	Ref< online::AttachmentResult > result = saveData->get(saveDataId);
	if (result->succeeded())
	{
		Ref< PropertyGroup > userSettingsGroup = dynamic_type_cast< PropertyGroup* >(result->get());
		if (userSettingsGroup)
			return new Settings(userSettingsGroup);
		else
			traktor::log::error << L"Failed to load settings; incorrect type of attachment" << Endl;
	}
	else
		traktor::log::error << L"Failed to load settings; unable to read save data" << Endl;

	return 0;
}

void saveSettings(IEnvironment* environment, const Settings* settings, const std::wstring& saveDataId, const online::SaveDataDesc& saveDataDesc, bool block)
{
	IOnlineServer* onlineServer = environment->getOnline();
	T_ASSERT (onlineServer);

	online::ISaveData* saveData = onlineServer->getSessionManager()->getSaveData();
	T_ASSERT (saveData);

	if (onlineServer && onlineServer->getSessionManager())
	{
		online::ISaveData* saveData = onlineServer->getSessionManager()->getSaveData();
		if (saveData)
		{
			Ref< online::Result > result = saveData->set(saveDataId, saveDataDesc, settings->getRootGroup(), true);
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

int32_t amalgamMain(
	const traktor::CommandLine& cmdLine,
	const std::wstring& settingsFileName,
	traktor::online::ISessionManagerProvider* sessionManagerProvider,
	amalgam::IStateFactory* stateFactory
)
{
	SystemCallback::getInstance().add(callbackExit);

	Path settingsPath = settingsFileName;
	if (cmdLine.getCount() >= 1)
		settingsPath = cmdLine.getString(0);

#if defined(T_MUTE_LOGS)
	traktor::log::debug.setBuffer(0);
	traktor::log::info.setBuffer(0);
	traktor::log::warning.setBuffer(0);
#endif

	traktor::log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< Settings > defaultSettings = loadSettings(settingsPath);
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

	Ref< Settings > settings = defaultSettings->clone();
	T_FATAL_ASSERT (settings);

	Ref< amalgam::Application > application = new amalgam::Application();
	if (application->create(
		defaultSettings,
		settings,
		sessionManagerProvider,
		stateFactory
	))
	{
		Environment* environment = checked_type_cast< Environment* >(application->getEnvironment());
		T_ASSERT (environment);

		IOnlineServer* onlineServer = environment->getOnline();

		// Load user settings from save-data; need to reconfigure environment if settings exist.
		if (!settingsSaveDataId.empty())
		{
			Ref< Settings > userSettings = loadSettings(environment, settingsSaveDataId);
			if (userSettings)
			{
				settings->merge(userSettings);
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

		application->destroy();
		application = 0;
	}

	traktor::log::info << L"Bye" << Endl;
	return 0;
}

	}
}

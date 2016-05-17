#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
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

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(int32_t level, const std::wstring& str)
	{
		(*m_stream) << str << Endl;
	}

private:
	Ref< OutputStream > m_stream;
};

class LogDualTarget : public ILogTarget
{
public:
	LogDualTarget(ILogTarget* target1, ILogTarget* target2)
	:	m_target1(target1)
	,	m_target2(target2)
	{
	}

	virtual void log(int32_t level, const std::wstring& str)
	{
		m_target1->log(level, str);
		m_target2->log(level, str);
	}

private:
	Ref< ILogTarget > m_target1;
	Ref< ILogTarget > m_target2;
};

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

void signalHandler(int sig)
{
	void* array[100];

	traktor::log::error << L"Error: signal " << sig << Endl;

	size_t size = backtrace(array, sizeof_array(array));
	char** strings = backtrace_symbols(array, size);
	if (strings)
	{
		for (size_t i = 0; i < size; ++i)
			traktor::log::error << mbstows(strings[i] ? strings[i] : "<null>") << Endl;
		free(strings);
	}

	exit(1);
}

}

int main(int argc, const char** argv)
{
	SystemApplication sysapp;

	// Install crash/exception signal handlers first of all things.
	signal(SIGILL, signalHandler);
	signal(SIGSEGV, signalHandler);

	// Initialize X11 thread primitives; thus must be performed very early.
	XInitThreads();

	// Let's continue with the application.
	CommandLine cmdLine(argc, argv);

	std::wstring writablePath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB";
	FileSystem::getInstance().makeAllDirectories(writablePath);

#if !defined(_DEBUG)
	Ref< IStream > logFile = FileSystem::getInstance().open(writablePath + L"/Application.log", File::FmWrite);
	if (logFile)
	{
		Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
		Ref< LogStreamTarget > logTarget = new LogStreamTarget(logStream);

		log::info   .setGlobalTarget(new LogDualTarget(logTarget, log::info   .getGlobalTarget()));
		log::warning.setGlobalTarget(new LogDualTarget(logTarget, log::warning.getGlobalTarget()));
		log::error  .setGlobalTarget(new LogDualTarget(logTarget, log::error  .getGlobalTarget()));

		log::info << L"Log file \"Application.log\" created" << Endl;
	}
	else
		log::error << L"Unable to create log file; logging only to std pipes" << Endl;
#endif

	Path settingsPath = L"Application.config";
	if (cmdLine.getCount() >= 1)
		settingsPath = cmdLine.getString(0);

	traktor::log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L"); please reinstall application" << Endl;
		traktor::log::error << L"Please reinstall application." << Endl;
		return 0;
	}

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	// Merge user settings into application settings.
	if (!cmdLine.hasOption('s', L"no-settings"))
	{
		Path userSettingsPath;
		Ref< PropertyGroup > userSettings;

		// First try to load user settings from current working directory; ie. same directory as
		// the main executable.
		userSettingsPath = settingsPath.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
		userSettings = loadSettings(userSettingsPath);

		// Try to load user settings from user's application data path; sometimes it's not possible
		// to store user settings alongside executable due to restrictive privileges.
		if (!userSettings)
		{
			userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
			userSettings = loadSettings(userSettingsPath);
		}

		if (userSettings)
			settings = settings->mergeReplace(userSettings);
	}

	if (!settings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		traktor::log::error << L"Please reinstall application." << Endl;
		return 1;
	}

	Ref< amalgam::Application > application = new amalgam::Application();
	if (application->create(
		defaultSettings,
		settings,
		sysapp,
		0
	))
	{
		// Enter main loop.
		try
		{
			for (;;)
			{
				if (!application->update())
				{
					traktor::log::info << L"Update returned false; terminating application..." << Endl;
					break;
				}
			}
		}
		catch(...)
		{
			traktor::log::error << L"Unhandled exception occured; terminating application..." << Endl;
		}

		safeDestroy(application);

		// Save user settings.
		if (!cmdLine.hasOption('s', L"no-settings"))
		{
			Path userSettingsPath = settingsPath.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
			if (!saveSettings(settings, userSettingsPath))
			{
				userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
				if (!saveSettings(settings, userSettingsPath))
					traktor::log::error << L"Unable to save user settings; user changes not saved" << Endl;
			}
		}
	}
	else
		traktor::log::error << L"Unable to create application" << Endl;

	traktor::log::info << L"Bye" << Endl;

#if !defined(_DEBUG)
	if (logFile)
	{
		traktor::log::info   .setGlobalTarget(0);
		traktor::log::warning.setGlobalTarget(0);
		traktor::log::error  .setGlobalTarget(0);

		logFile->close();
		logFile = 0;
	}
#endif
	return 0;
}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <Cocoa/Cocoa.h>

#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Runtime/App/OsX/ErrorDialog.h"
#include "Runtime/Impl/Application.h"
#include "Ui/Application.h"
#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Cocoa/WidgetFactoryCocoa.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

using namespace traktor;

namespace
{

class LogTailTarget : public ILogTarget
{
public:
	Semaphore m_lock;
	std::list< std::wstring > m_tail;

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (m_tail.size() > 100)
			m_tail.pop_front();
		m_tail.push_back(str);
	}
};

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		(*m_stream) << L"[" << DateTime::now().format(L"%H:%M:%S") << L"] " << str << Endl;
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

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		m_target1->log(threadId, level, str);
		m_target2->log(threadId, level, str);
	}

private:
	Ref< ILogTarget > m_target1;
	Ref< ILogTarget > m_target2;
};

Ref< PropertyGroup > loadSettings(const Path& settingsFile)
{
	Ref< PropertyGroup > settings;

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file, settingsFile.getPathName()).readObject< PropertyGroup >();
		file->close();
	}

	return settings;
}

bool saveSettings(const PropertyGroup* settings, const Path& settingsFile)
{
	T_ASSERT (settings);

	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmWrite);
	if (!file)
		return false;

	bool result = xml::XmlSerializer(file).writeObject(settings);
	file->close();

	return result;
}

void showErrorDialog(const std::list< std::wstring >& tail)
{
	amalgam::ErrorDialog errorDialog;
	if (errorDialog.create())
	{
		for (const auto& error : tail)
			errorDialog.addErrorString(error);

		errorDialog.showModal();
		errorDialog.destroy();
	}
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	SystemApplication sysapp;
	Ref< traktor::IStream > logFile;

	std::wstring writablePath = OS::getInstance().getWritableFolderPath() + L"/Traktor";
	FileSystem::getInstance().makeAllDirectories(writablePath);

#if !defined(_DEBUG)
	RefArray< File > logs;
	FileSystem::getInstance().find(writablePath + L"/Application_*.log", logs);

	// Get "alive" log ids.
	std::vector< int32_t > logIds;
	for (RefArray< File >::const_iterator i = logs.begin(); i != logs.end(); ++i)
	{
		std::wstring logName = (*i)->getPath().getFileNameNoExtension();
		size_t p = logName.find(L'_');
		if (p != logName.npos)
		{
			int32_t id = parseString< int32_t >(logName.substr(p + 1), -1);
			if (id != -1)
				logIds.push_back(id);
		}
	}

	int32_t nextLogId = 0;
	if (!logIds.empty())
	{
		std::sort(logIds.begin(), logIds.end());

		// Don't keep more than 10 log files.
		while (logIds.size() >= 10)
		{
			StringOutputStream ss;
			ss << writablePath << L"/Application_" << logIds.front() << L".log";
			FileSystem::getInstance().remove(ss.str());
			logIds.erase(logIds.begin());
		}

		nextLogId = logIds.back() + 1;
	}

	// Create new log file.
	StringOutputStream ss;
	ss << writablePath << L"/Application_" << nextLogId << L".log";
	logFile = FileSystem::getInstance().open(ss.str(), File::FmWrite);
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

	Ref< LogTailTarget > logTail = new LogTailTarget();
	log::info   .setGlobalTarget(new LogDualTarget(logTail, log::info   .getGlobalTarget()));
	log::warning.setGlobalTarget(new LogDualTarget(logTail, log::warning.getGlobalTarget()));
	log::error  .setGlobalTarget(new LogDualTarget(logTail, log::error  .getGlobalTarget()));

	// Initialize native UI.
	ui::Application::getInstance()->initialize(
		new ui::WidgetFactoryCocoa(),
		nullptr
	);

	Path settingsPath = L"$(BUNDLE_PATH)/Contents/Resources/Application.config";

	// Override settings path either from command line or application bundle.
	if (cmdLine.getCount() >= 1)
		settingsPath = cmdLine.getString(0);
	else
	{
		// Set default path to resources in bundle.
		FileSystem::getInstance().setCurrentVolumeAndDirectory(L"$(BUNDLE_PATH)/Contents/Resources");

		// Get optional configuration filename from Info.plist or environment.
		std::wstring tmp;
		if (OS::getInstance().getEnvironment(L"DEAConfiguration", tmp))
			settingsPath = tmp;
	}

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		log::error << L"Please reinstall application." << Endl;
		showErrorDialog(logTail->m_tail);
		return 1;
	}

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	// Merge user settings into application settings.
	if (!cmdLine.hasOption('s', L"no-settings"))
	{
		Path userSettingsPath;
		Ref< PropertyGroup > userSettings;

		// Try to load user settings from user's application data path; sometimes it's not possible
		// to store user settings alongside executable due to restrictive privileges.
		if (!userSettings)
		{
			userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
			userSettings = loadSettings(userSettingsPath);
		}

		if (userSettings)
			settings = settings->merge(userSettings, PropertyGroup::MmReplace);
	}

	if (!settings)
	{
		log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		log::error << L"Please reinstall application." << Endl;
		showErrorDialog(logTail->m_tail);
		return 1;
	}

	// Create runtime application.
	Ref< runtime::Application > application = new runtime::Application();
	if (application->create(
		defaultSettings,
		settings,
		sysapp,
		nullptr
	))
	{
		for (;;)
		{
			// Handle system events.
			for (;;)
			{
				// Pop event from queue.
				NSEvent* event = [NSApp nextEventMatchingMask: NSEventMaskAny untilDate: nil inMode: NSDefaultRunLoopMode dequeue: YES];
				if (event == nil)
					break;

				// Dispatch event to appropriate responder.
				[NSApp sendEvent: event];
			}

			// Update game application.
			if (!application->update())
				break;
		}

		safeDestroy(application);

		// Save user settings.
		if (!cmdLine.hasOption('s', L"no-settings"))
		{
			Path userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
			if (!saveSettings(settings, userSettingsPath))
				log::error << L"Unable to save user settings; user changes not saved." << Endl;
		}
	}
	else
	{
		safeDestroy(application);
		showErrorDialog(logTail->m_tail);
	}

	settings = nullptr;
	defaultSettings = nullptr;

	ui::Application::getInstance()->finalize();

#if !defined(_DEBUG)
	if (logFile)
	{
		logFile->close();
		logFile = nullptr;
	}
#endif

	log::info   .setGlobalTarget(nullptr);
	log::warning.setGlobalTarget(nullptr);
	log::error  .setGlobalTarget(nullptr);

#if defined(_DEBUG)
	SingletonManager::getInstance().destroy();
#endif
	return 0;
}

#include "Amalgam/App/MacOSX/ErrorDialog.h"
#include "Amalgam/Impl/Application.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
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

	virtual void log(int32_t level, const std::wstring& str)
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

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
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
		for (std::list< std::wstring >::const_iterator i = tail.begin(); i != tail.end(); ++i)
			errorDialog.addErrorString(*i);
			
		errorDialog.addErrorString(L"Please copy this information and contact");
		errorDialog.addErrorString(L"support@doctorentertainment.com");

		errorDialog.showModal();
		errorDialog.destroy();
	}
}

}

int main(int argc, const char** argv)
{
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

	Ref< LogTailTarget > logTail = new LogTailTarget();
	log::info   .setGlobalTarget(new LogDualTarget(logTail, log::info   .getGlobalTarget()));
	log::warning.setGlobalTarget(new LogDualTarget(logTail, log::warning.getGlobalTarget()));
	log::error  .setGlobalTarget(new LogDualTarget(logTail, log::error  .getGlobalTarget()));

	// Initialize native UI.
	ui::Application::getInstance()->initialize(
		new ui::EventLoopCocoa(),
		new ui::WidgetFactoryCocoa()
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
			settings = settings->mergeReplace(userSettings);
	}

	if (!settings)
	{
		log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		log::error << L"Please reinstall application." << Endl;
		showErrorDialog(logTail->m_tail);
		return 1;
	}

	// Create amalgam application.
	Ref< amalgam::Application > application = new amalgam::Application();
	if (application->create(
		defaultSettings,
		settings,
		0,
		0
	))
	{
		for (;;)
		{
			if (!application->update())
				break;
		}

		safeDestroy(application);

		// Save user settings.
		if (!cmdLine.hasOption('s', L"no-settings"))
		{
			Path userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
			if (!saveSettings(settings, userSettingsPath))
				log::error << L"Unable to save user settings; user changes not saved" << Endl;
		}
	}
	else
	{
		safeDestroy(application);
		showErrorDialog(logTail->m_tail);
	}
	
	settings = 0;
	defaultSettings = 0;

	ui::Application::getInstance()->finalize();

#if !defined(_DEBUG)
	if (logFile)
	{
		log::info   .setGlobalTarget(0);
		log::warning.setGlobalTarget(0);
		log::error  .setGlobalTarget(0);

		logFile->close();
		logFile = 0;
	}
#endif

	return 0;
}

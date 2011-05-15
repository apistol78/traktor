#include "Core/Date/DateTime.h"
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
#include "Core/Settings/Settings.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Net/Network.h"
#include "Net/Url.h"
#include "Ui/Application.h"
#if defined(_WIN32)
#	include "Ui/Win32/EventLoopWin32.h"
#	include "Ui/Win32/WidgetFactoryWin32.h"
#elif defined(__APPLE__)
#	include "Ui/Cocoa/EventLoopCocoa.h"
#	include "Ui/Cocoa/WidgetFactoryCocoa.h"
#endif
#include "Update/Process.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"
#include "Amalgam/Launch.h"
#include "Amalgam/Impl/Application.h"
#include "Amalgam/Impl/Std/ErrorDialog.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

class LogTailTarget : public ILogTarget
{
public:
	Semaphore m_lock;
	std::list< std::wstring > m_tail;

	virtual void log(const std::wstring& str)
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

	virtual void log(const std::wstring& str)
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

	virtual void log(const std::wstring& str)
	{
		m_target1->log(str);
		m_target2->log(str);
	}

private:
	Ref< ILogTarget > m_target1;
	Ref< ILogTarget > m_target2;
};
/*
class LogStampTarget : public ILogTarget
{
public:
	LogStampTarget(ILogTarget* target)
	:	m_target(target)
	{
	}
	
	virtual void log(const std::wstring& str)
	{
		StringOutputStream ss;
		ss << DateTime::now().format(L"%Y-%m-%d %H:%M:%S") << L" - " << str;
		m_target->log(ss.str());
	}
	
private:
	Ref< ILogTarget > m_target;
};
*/
Ref< Settings > loadSettings(const Path& settingsFile)
{
	Ref< Settings > settings;

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}

	return settings;
}

void saveSettings(const Settings* settings, const Path& settingsFile)
{
	T_ASSERT (settings);

	Ref< IStream > file = FileSystem::getInstance().open(settingsFile, File::FmWrite);
	if (file)
	{
		settings->write< xml::XmlSerializer >(file);
		file->close();
	}
}

void showErrorDialog(const std::list< std::wstring >& tail)
{
	ErrorDialog errorDialog;
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

bool checkPreconditions()
{
#if defined(_WIN32)

	BOOL sseSupported = ::IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
	if (!sseSupported)
	{
		log::error << L"This game requires a CPU with SSE support" << Endl;
		return false;
	}

#endif
	return true;
}

}

int32_t amalgamMain(
	const CommandLine& cmdLine,
	const std::wstring& settingsFileName,
	online::ISessionManagerProvider* sessionManagerProvider,
	IStateFactory* stateFactory
)
{
	if (!checkPreconditions())
		return 1;

#if !defined(_DEBUG)
#	if defined(__APPLE__)
	Ref< IStream > logFile = FileSystem::getInstance().open(L"$(BUNDLE_PATH)/Contents/Resources/Application.log", File::FmWrite);
#	else
	Ref< IStream > logFile = FileSystem::getInstance().open(L"Application.log", File::FmWrite);
#	endif
	if (logFile)
	{
		Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
		Ref< LogStreamTarget > /*logStreamTarget*/logTarget = new LogStreamTarget(logStream);
		//Ref< LogStampTarget > logTarget = new LogStampTarget(logStreamTarget);

		log::info   .setTarget(logTarget);
		log::warning.setTarget(logTarget);
		log::error  .setTarget(logTarget);

		log::info << L"Log file \"Application.log\" created" << Endl;
	}
	else
		log::error << L"Unable to create log file; logging only to std pipes" << Endl;
#endif

	Ref< LogTailTarget > logTail = new LogTailTarget();
	log::info   .setTarget(new LogDualTarget(logTail, log::info   .getTarget()));
	log::warning.setTarget(new LogDualTarget(logTail, log::warning.getTarget()));
	log::error  .setTarget(new LogDualTarget(logTail, log::error  .getTarget()));

	// Initialize native UI.
#if defined(_WIN32)
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32()
	);
#elif defined(__APPLE__)
	ui::Application::getInstance()->initialize(
		new ui::EventLoopCocoa(),
		new ui::WidgetFactoryCocoa()
	);
#endif

	// Initialize networking.
	net::Network::initialize();

#if !defined(__APPLE__)
	Path settingsPath = settingsFileName;
#else
	Path settingsPath = L"$(BUNDLE_PATH)/Contents/Resources/" + settingsFileName;
#endif

	// Override settings path either from command line or application bundle.
	if (cmdLine.getCount() >= 1)
		settingsPath = cmdLine.getString(0);
#if defined(__APPLE__)
	else
	{
		std::wstring tmp;
		if (OS::getInstance().getEnvironment(L"DEAConfiguration", tmp))
			settingsPath = tmp;
	}
#endif

	Ref< Settings > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		log::error << L"Please reinstall application." << Endl;
		showErrorDialog(logTail->m_tail);
		return 1;
	}

	Ref< Settings > settings = defaultSettings->clone();
	T_FATAL_ASSERT (settings);

	// Merge user settings into application settings.
	Path userSettingsPath = settingsPath.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
	if (!cmdLine.hasOption('s'))
	{
		Ref< Settings > userSettings = loadSettings(userSettingsPath);
		if (userSettings)
			settings->merge(userSettings, false);
	}

	if (!settings)
	{
		log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		log::error << L"Please reinstall application." << Endl;
		showErrorDialog(logTail->m_tail);
		return 1;
	}

	// Check if update is available.
	if (
		defaultSettings->getProperty< PropertyBoolean >(L"Amalgam.AutoUpdateEnable", true) &&
		!cmdLine.hasOption(L'u')
	)
	{
		net::Url url = defaultSettings->getProperty< PropertyString >(L"Amalgam.AutoUpdateUrl");
		if (url.valid())
		{
			update::Process updateProcess;
			update::Process::CheckResult result = updateProcess.check(url);
			if (result == update::Process::CrTerminate)
			{
				log::info << L"Updates ready; restarting..." << Endl;
				return 0;
			}
		}
	}

	// Create amalgam application.
	Ref< amalgam::Application > application = new amalgam::Application();
	if (application->create(
		defaultSettings,
		settings,
		sessionManagerProvider,
		stateFactory,
		0
	))
	{
		for (;;)
		{
			if (!application->update())
				break;
		}

		safeDestroy(application);

		if (!cmdLine.hasOption('s'))
			saveSettings(settings, userSettingsPath);
	}
	else
	{
		safeDestroy(application);
		showErrorDialog(logTail->m_tail);
	}
	settings = 0;
	defaultSettings = 0;

	net::Network::finalize();

	ui::Application::getInstance()->finalize();

#if !defined(_DEBUG)
	if (logFile)
	{
		log::info   .setTarget(0);
		log::warning.setTarget(0);
		log::error  .setTarget(0);

		logFile->close();
		logFile = 0;
	}
#endif

	return 0;
}

	}
}

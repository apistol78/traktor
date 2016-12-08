#include <jni.h>
#include <android_native_app_glue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "Amalgam/Game/IOnlineServer.h"
#include "Amalgam/Game/IRenderServer.h"
#include "Amalgam/Game/Impl/Application.h"
#include "Amalgam/Game/Impl/Environment.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Io/Android/AssetsVolume.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/System/OS.h"
#include "Core/System/Android/DelegateInstance.h"
#include "Render/IRenderView.h"
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

class AndroidApplication : public DelegateInstance
{
public:
	AndroidApplication(struct android_app* app);

	bool readSettings();

	bool createApplication();

	void destroyApplication();

	bool updateApplication();

	void suspendApplication();

	void resumeApplication();

	virtual struct android_app* getApplication() T_OVERRIDE T_FINAL;

	virtual struct ANativeActivity* getActivity() T_OVERRIDE T_FINAL;

	virtual void handleCommand(int32_t cmd) T_OVERRIDE T_FINAL;

private:
	struct android_app* m_app;
	SystemWindow m_syswin;
	SystemApplication m_sysapp;
	Ref< const PropertyGroup > m_defaultSettings;
	Ref< PropertyGroup > m_settings;
	Ref< amalgam::Application > m_application;
	bool m_suspended;
};

AndroidApplication::AndroidApplication(struct android_app* app)
:	m_app(app)
,	m_syswin(&app->window)
,	m_sysapp(this)
,	m_suspended(false)
{
}

bool AndroidApplication::readSettings()
{
	Path settingsPath = L"Application.config";
	log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L"); please reinstall application" << Endl;
		log::error << L"Please reinstall application." << Endl;
		return false;
	}

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	if (!settings)
	{
		log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		log::error << L"Please reinstall application." << Endl;
		return false;
	}

	m_defaultSettings = defaultSettings;
	m_settings = settings;

	return true;
}

bool AndroidApplication::createApplication()
{
	m_application = new amalgam::Application();
	if (m_application->create(
		m_defaultSettings,
		m_settings,
		m_sysapp,
		&m_syswin
	))
		return true;
	else
	{
		safeDestroy(m_application);
		return false;
	}
}

void AndroidApplication::destroyApplication()
{
	safeDestroy(m_application);
}

bool AndroidApplication::updateApplication()
{
	if (m_application && !m_suspended)
		return m_application->update();
	else
		return true;
}

void AndroidApplication::suspendApplication()
{
	if (!m_suspended)
	{
		if (m_application)
			m_application->suspend();

		m_suspended = true;
	}
}

void AndroidApplication::resumeApplication()
{
	if (m_suspended)
	{
		if (m_application)
		{
			m_application->getEnvironment()->getRender()->getRenderView()->reset(0, 0);
			m_application->resume();
		}
		m_suspended = false;
	}
}

struct android_app* AndroidApplication::getApplication()
{
	return m_app;
}

struct ANativeActivity* AndroidApplication::getActivity()
{
	return m_app->activity;
}

void AndroidApplication::handleCommand(int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_INPUT_CHANGED:
		log::debug << L"handleCommand APP_CMD_INPUT_CHANGED" << Endl;
		break;

	case APP_CMD_INIT_WINDOW:
		log::debug << L"handleCommand APP_CMD_INIT_WINDOW (window: " << int32_t(m_app->window) << L")" << Endl;
		if (!m_application && m_app->window != 0)
			createApplication();
		break;

	case APP_CMD_TERM_WINDOW:
		log::debug << L"handleCommand APP_CMD_TERM_WINDOW" << Endl;
		suspendApplication();
		break;

	case APP_CMD_WINDOW_RESIZED:
		log::debug << L"handleCommand APP_CMD_WINDOW_RESIZED" << Endl;
		break;

	case APP_CMD_WINDOW_REDRAW_NEEDED:
		log::debug << L"handleCommand APP_CMD_WINDOW_REDRAW_NEEDED" << Endl;
		break;

	case APP_CMD_CONTENT_RECT_CHANGED:
		log::debug << L"handleCommand APP_CMD_CONTENT_RECT_CHANGED" << Endl;
		break;

	case APP_CMD_GAINED_FOCUS:
		log::debug << L"handleCommand APP_CMD_GAINED_FOCUS" << Endl;
		resumeApplication();
		break;

	case APP_CMD_LOST_FOCUS:
		log::debug << L"handleCommand APP_CMD_LOST_FOCUS" << Endl;
		suspendApplication();
		break;

	case APP_CMD_CONFIG_CHANGED:
		log::debug << L"handleCommand APP_CMD_CONFIG_CHANGED" << Endl;
		break;

	case APP_CMD_LOW_MEMORY:
		log::debug << L"handleCommand APP_CMD_LOW_MEMORY" << Endl;
		break;

	case APP_CMD_START:
		log::debug << L"handleCommand APP_CMD_START" << Endl;
		break;

	case APP_CMD_RESUME:
		log::debug << L"handleCommand APP_CMD_RESUME" << Endl;
		break;

	case APP_CMD_SAVE_STATE:
		log::debug << L"handleCommand APP_CMD_SAVE_STATE" << Endl;
		break;

	case APP_CMD_PAUSE:
		log::debug << L"handleCommand APP_CMD_PAUSE" << Endl;
		break;

	case APP_CMD_STOP:
		log::debug << L"handleCommand APP_CMD_STOP" << Endl;
		break;

	case APP_CMD_DESTROY:
		log::debug << L"handleCommand APP_CMD_DESTROY" << Endl;
		destroyApplication();
		break;

	default:
		log::warning << L"handleCommand <Unknown command>" << Endl;
		break;
	}
	DelegateInstance::handleCommand(cmd);
}

// Android ============================

void handleCommand(struct android_app* app, int32_t cmd)
{
	AndroidApplication* aa = (AndroidApplication*)app->userData;
	aa->handleCommand(cmd);
}

int32_t handleInput(struct android_app* app, AInputEvent* event)
{
	AndroidApplication* aa = (AndroidApplication*)app->userData;
	aa->handleInput(event);
	return 0;
}

extern "C" void traktor_main(struct android_app* state)
{
	AndroidApplication aa(state);

	// Expose application's data paths in our environment.
	setenv("INTERNAL_DATA_PATH", state->activity->internalDataPath, 1); 
	setenv("EXTERNAL_DATA_PATH", state->activity->externalDataPath, 1); 

#if defined(_DEBUG)
	log::info << L"Using following data paths," << Endl;
	log::info << L"\tINTERNAL_DATA_PATH = \"" << mbstows(state->activity->internalDataPath) << L"\"" << Endl;
	log::info << L"\tEXTERNAL_DATA_PATH = \"" << mbstows(state->activity->externalDataPath) << L"\"" << Endl;
#endif

	mkdir(state->activity->internalDataPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	mkdir(state->activity->externalDataPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	app_dummy();
	FileSystem::getInstance().mount(L"assets", new AssetsVolume(state->activity));
	FileSystem::getInstance().setCurrentVolumeAndDirectory(L"assets:");

	state->userData = &aa;
	state->onAppCmd = handleCommand;
	state->onInputEvent = handleInput;

	if (!aa.readSettings())
		return;

	struct android_poll_source* source;
	int ident;
	int events;

	for (;;)
	{
		while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
		{
			if (source != NULL)
				source->process(state, source);

			aa.handleEvents();
		}

		if (!aa.updateApplication())
			break;
	}
}

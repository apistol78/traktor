#include <jni.h>
#include <android_native_app_glue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "Amalgam/Game/IOnlineServer.h"
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
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"
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

void updateApplicationThread(amalgam::Application* app)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped())
	{
		if (!app->update())
			break;
	}
}

}

class AndroidApplication : public DelegateInstance
{
public:
	AndroidApplication(struct android_app* app);

	bool readSettings();

	bool createApplication();

	void destroyApplication();

	//bool updateApplication();

	void startAnimation();

	void stopAnimation();

	virtual void handleCommand(struct android_app* app, int32_t cmd);

private:
	struct android_app* m_app;
	Ref< const PropertyGroup > m_defaultSettings;
	Ref< PropertyGroup > m_settings;
	Ref< amalgam::Application > m_application;
	Thread* m_thread;
};

AndroidApplication::AndroidApplication(struct android_app* app)
:	m_app(app)
,	m_thread(0)
{
}

bool AndroidApplication::readSettings()
{
	Path settingsPath = L"Application.config";
	traktor::log::info << L"Using settings \"" << settingsPath.getPathName() << L"\"" << Endl;

	Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L"); please reinstall application" << Endl;
		traktor::log::error << L"Please reinstall application." << Endl;
		return false;
	}

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	if (!settings)
	{
		traktor::log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
		traktor::log::error << L"Please reinstall application." << Endl;
		return false;
	}

	m_defaultSettings = defaultSettings;
	m_settings = settings;

	return true;
}

bool AndroidApplication::createApplication()
{
	m_application = new amalgam::Application();
	return m_application->create(
		m_defaultSettings,
		m_settings,
		this,
		m_app->window
	);
}

void AndroidApplication::destroyApplication()
{
	safeDestroy(m_application);
}

void AndroidApplication::startAnimation()
{
	if (!m_thread)
	{
		m_thread = ThreadManager::getInstance().create(
			makeStaticFunctor(updateApplicationThread, m_application.ptr()),
			L"Application update thread"
		);
		m_thread->start();
	}
}

void AndroidApplication::stopAnimation()
{
	if (m_thread)
	{
		m_thread->stop();
		m_thread = 0;
	}
}

//bool AndroidApplication::updateApplication()
//{
//	if (m_application)
//		return m_application->update();
//	else
//		return true;
//}

void AndroidApplication::handleCommand(struct android_app* app, int32_t cmd)
{
	T_ASSERT (app == m_app);

	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		if (app->window != 0)
		{
			log::info << L"APP_CMD_INIT_WINDOW" << Endl;
			createApplication();
			startAnimation();
		}
		break;

	case APP_CMD_TERM_WINDOW:
		log::info << L"APP_CMD_TERM_WINDOW" << Endl;
		stopAnimation();
		destroyApplication();
		break;

	case APP_CMD_GAINED_FOCUS:
		log::info << L"APP_CMD_GAINED_FOCUS" << Endl;
		startAnimation();
		break;

	case APP_CMD_LOST_FOCUS:
		log::info << L"APP_CMD_LOST_FOCUS" << Endl;
		stopAnimation();
		break;
	}

	DelegateInstance::handleCommand(app, cmd);
}

// Android ============================

void handleCommand(struct android_app* app, int32_t cmd)
{
	AndroidApplication* aa = (AndroidApplication*)app->userData;
	aa->handleCommand(app, cmd);
}

int32_t handleInput(struct android_app* app, AInputEvent* event)
{
	AndroidApplication* aa = (AndroidApplication*)app->userData;
	aa->handleInput(app, event);
	return 0;
}

void android_main(struct android_app* state)
{
	AndroidApplication aa(state);

	// Expose application's data paths in our environment.
	setenv("INTERNAL_DATA_PATH", state->activity->internalDataPath, 1); 
	setenv("EXTERNAL_DATA_PATH", state->activity->externalDataPath, 1); 

	log::info << L"Using following data paths," << Endl;
	log::info << L"\tINTERNAL_DATA_PATH = \"" << mbstows(state->activity->internalDataPath) << L"\"" << Endl;
	log::info << L"\tEXTERNAL_DATA_PATH = \"" << mbstows(state->activity->externalDataPath) << L"\"" << Endl;

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

	{
		struct android_poll_source* source;
		int ident;
		int events;

		for (;;)
		{
			while ((ident = ALooper_pollAll(100, NULL, &events, (void**)&source)) >= 0)
			{
				if (source != NULL)
					source->process(state, source);
			}
			//if (!aa.updateApplication())
			//	break;
		}
	}
}

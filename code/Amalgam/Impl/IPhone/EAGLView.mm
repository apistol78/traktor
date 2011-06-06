#import "Amalgam/Impl/IPhone/EAGLView.h"

#include "Amalgam/Impl/Application.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Settings/Settings.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace traktor
{
	namespace amalgam
	{
	
extern const CommandLine* g_cmdLine;
extern traktor::online::ISessionManagerProvider* g_sessionManagerProvider;
extern traktor::amalgam::IStateFactory* g_stateFactory;

	}
}

namespace
{

Ref< Settings > loadSettings(const Path& settingsPath)
{
	Ref< Settings > settings;
	Ref< IStream > file = FileSystem::getInstance().open(settingsPath, File::FmRead);
	if (file)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}
	return settings;
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

@interface EAGLView ()
@end

@implementation EAGLView

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (id) initWithFrame: (CGRect)frame
{    
    if ((self = [super initWithFrame:frame]))
	{
		m_thread = 0;

		// Load settings.
		Path settingsPath = L"$(BUNDLE_PATH)/Application.config";
		Ref< Settings > defaultSettings = loadSettings(settingsPath);
		if (!defaultSettings)
		{
			traktor::log::error << L"Unable to read application settings \"" << settingsPath.getPathName() << L"\"" << Endl;
			return 0;
		}
		
		Ref< Settings > settings = defaultSettings->clone();
		T_FATAL_ASSERT (settings);

		// Create application.
		m_application = new amalgam::Application();
		if (!m_application->create(
			defaultSettings,
			settings,
			traktor::amalgam::g_sessionManagerProvider,
			traktor::amalgam::g_stateFactory,
			(void*)self
		))
			return 0;
    }
	
    return self;
}

- (void) drawView:(id)sender
{
}

- (void) layoutSubviews
{
}

- (void) startAnimation
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

- (void)stopAnimation
{
	if (m_thread)
	{
		m_thread->stop();
		m_thread = 0;
	}
}

- (void) dealloc
{
	if (m_application)
	{
		m_application->destroy();
		m_application = 0;
	}

    [super dealloc];
}

@end

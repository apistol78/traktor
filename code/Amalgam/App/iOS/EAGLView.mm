#import "Amalgam/App/iOS/EAGLView.h"

#include "Amalgam/Impl/Application.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

Ref< PropertyGroup > loadSettings(const Path& settingsPath)
{
	Ref< PropertyGroup > settings;
	Ref< IStream > file = FileSystem::getInstance().open(settingsPath, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}
	return settings;
}

uint32_t g_runMode = 0;
uint32_t g_runModePending = 0;

void updateApplicationThread(amalgam::Application* app)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped())
	{
		if (g_runMode != g_runModePending)
		{
			if (g_runModePending == 0)
				app->suspend();
			else if (g_runModePending == 1)
				app->resume();

			g_runMode = g_runModePending;
		}

		if (g_runMode == 1)
		{
			if (!app->update())
				break;
		}
		else
			currentThread->sleep(100);
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

- (BOOL) createApplication
{
	m_thread = 0;

	// Load settings.
	Ref< PropertyGroup > defaultSettings = loadSettings(L"Application.config");
	if (!defaultSettings)
	{
		traktor::log::error << L"Unable to read application settings \"Application.config\"" << Endl;
		return NO;
	}

	// "Activate" retina display if application want's to use it.
	if (defaultSettings->getProperty< PropertyBoolean >(L"Amalgam.SupportRetina", false))
	{
		// Adjust scale as we want full resolution of a retina display.
		float scale = [UIScreen mainScreen].scale;

		// Set contentScale Factor to 2.
		self.contentScaleFactor = scale;

		// Also set our glLayer contentScale Factor to 2.
		CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
		eaglLayer.contentsScale = scale;
	}

	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	// Create application.
	m_application = new amalgam::Application();
	if (!m_application->create(
		defaultSettings,
		settings,
        0,
		(void*)self
	))
		return NO;

	// Create update thread.
	m_thread = ThreadManager::getInstance().create(
		makeStaticFunctor(updateApplicationThread, m_application.ptr()),
		L"Application update thread"
	);
	m_thread->start();
	return YES;
}

- (void) drawView:(id)sender
{
}

- (void) layoutSubviews
{
}

- (void) startAnimation
{
	g_runModePending = 1;
	while (g_runMode != g_runModePending)
		ThreadManager::getInstance().getCurrentThread()->sleep(10);
}

- (void)stopAnimation
{
	g_runModePending = 0;
	while (g_runMode != g_runModePending)
		ThreadManager::getInstance().getCurrentThread()->sleep(10);
}

- (void) dealloc
{
	[self stopAnimation];

	if (m_thread)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = 0;
	}
	
	if (m_application)
	{
		m_application->destroy();
		m_application = 0;
	}

    [super dealloc];
}

@end

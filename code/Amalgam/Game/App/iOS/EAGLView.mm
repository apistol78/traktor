#include <algorithm>
#include <dispatch/dispatch.h>

#import "Amalgam/Game/App/iOS/EAGLView.h"

#include "Amalgam/Game/Impl/Application.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
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

bool g_animation = false;
bool g_suspend[2] = { false, true };

void updateApplicationThread(Ref< PropertyGroup > defaultSettings, EAGLView* view)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();

	// As we doesn't need to store user defined settings on iOS we
	// create a plain copy of the default settings.
	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	// Create application.
	Ref< amalgam::Application > application = new amalgam::Application();
	if (!application->create(
		defaultSettings,
		settings,
        0,
		(void*)view
	))
		return;

	// Enter update loop.
	while (!currentThread->stopped())
	{
		// Should we suspend/resume?
		if (g_suspend[0] != g_suspend[1])
		{
			if (g_suspend[0])
				application->suspend();
			else
				application->resume();
			g_suspend[1] = g_suspend[0];
		}

		// Animation update.
		if (g_animation && !g_suspend[1])
		{
			if (!application->update())
				break;
		}
		else
			currentThread->sleep(100);
	}

	// Destroy application.
	safeDestroy(application);
}

}

@interface EAGLView ()
{    
@private
	traktor::Thread* m_thread;
}

@end

@implementation EAGLView

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (BOOL) createApplication
{
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
		float scale = 1.0f;

		// Primarily use "nativeScale" as we don't want iPhone 6+ downscaling;
		// in case non iOS 8 we use ordinary "scale" property.
		if ([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
			scale = [UIScreen mainScreen].nativeScale;
		else
			scale = [UIScreen mainScreen].scale;

		self.contentScaleFactor = scale;

		CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
		if (eaglLayer)
			eaglLayer.contentsScale = scale;
	}

	// Create application thread; it will first start loading resources
	// before settling on frequent updating application.
	m_thread = ThreadManager::getInstance().create(
		makeStaticFunctor< Ref< PropertyGroup >, EAGLView* >(updateApplicationThread, defaultSettings, self),
		L"Application update thread"
	);
	if (!m_thread)
		return NO;

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
	g_animation = true;
}

- (void) stopAnimation
{
	g_animation = false;
}

- (void) suspend
{
	g_suspend[0] = true;
	while (m_thread && g_suspend[0] != g_suspend[1])
		ThreadManager::getInstance().getCurrentThread()->sleep(10);
}

- (void) resume
{
	g_suspend[0] = false;
	while (m_thread && g_suspend[0] != g_suspend[1])
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

    [super dealloc];
}

@end

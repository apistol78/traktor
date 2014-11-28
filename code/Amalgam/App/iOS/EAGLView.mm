#include <dispatch/dispatch.h>

#import "Amalgam/App/iOS/EAGLView.h"

#include "Amalgam/Impl/Application.h"
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

uint32_t g_runMode = 0;
uint32_t g_runModePending = 0;

void updateApplicationThread(Ref< PropertyGroup > defaultSettings, EAGLView* view)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	Timer timer;

	// As we doesn't need to store user defined settings on iOS we
	// create a plain copy of the default settings.
	Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
	T_FATAL_ASSERT (settings);

	// Start splash timer.
	timer.start();

	// Create application.
	Ref< amalgam::Application > application = new amalgam::Application();
	if (!application->create(
		defaultSettings,
		settings,
        0,
		(void*)view
	))
		return;

	// Ensure splash has been shown atleast four seconds;
	// it's a requirement of publisher.
	double Tend = timer.getElapsedTime();
	if (Tend < 4.0)
		currentThread->sleep(int32_t((4.0 - Tend) * 1000.0));

	// Enter update loop.
	while (!currentThread->stopped())
	{
		if (g_runMode != g_runModePending)
		{
			if (g_runModePending == 0)
			{
				application->suspend();
			}
			else if (g_runModePending == 1)
			{
				application->resume();

				// Finished creating/resuming application; let view know so
				// it can hide the splash screen.
				dispatch_sync(dispatch_get_main_queue(), ^{
					[view hideSplash];
				});
			}

			g_runMode = g_runModePending;
		}

		if (g_runMode == 1)
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
	UIImageView* m_splashView;
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
		// Use "nativeScale" as we don't want iPhone 6+ downscaling.
		float scale = [UIScreen mainScreen].nativeScale;
		self.contentScaleFactor = scale;

		CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
		eaglLayer.contentsScale = scale;
	}

	// Show splash image until application has finished loading all resources.
	[self showSplash];

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

- (void) showSplash
{
	if (m_splashView == nil)
	{
		CGRect screenRect = [[UIScreen mainScreen] bounds];
		float screenWidth = screenRect.size.width;
		float screenHeight = screenRect.size.height;

		UIImage* splashImage = nil;

		if (screenHeight == 568.0f)
			splashImage = [UIImage imageNamed: @"Default-568h"];
		else if (screenHeight == 667.0f)
			splashImage = [UIImage imageNamed: @"Default-667h"];
		
		if (!splashImage)
			splashImage = [UIImage imageNamed: @"Default"];

		if (splashImage)
		{
			m_splashView = [[UIImageView alloc] initWithFrame: CGRectMake(0, 0, screenWidth, screenHeight)];
			m_splashView.image = splashImage;

			[self addSubview: m_splashView];
		}
	}
}

- (void) hideSplash
{
	if (m_splashView != nil)
	{
		[m_splashView removeFromSuperview];
		[m_splashView dealloc];
		m_splashView = nil;
	}
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
}

- (void) stopAnimation
{
	g_runModePending = 0;
}

- (void) waitUntilRunMode
{
	while (m_thread && g_runMode != g_runModePending)
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

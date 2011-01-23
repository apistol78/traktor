#import "Amalgam/Impl/IPhone/EAGLView.h"

#include "Amalgam/Impl/Application.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Settings/Settings.h"
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

}

@implementation EAGLView

@synthesize animating;
@dynamic animationFrameInterval;

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

- (id) initWithCoder:(NSCoder*)coder
{    
    if ((self = [super initWithCoder:coder]))
	{
		animating = FALSE;
		displayLinkSupported = FALSE;
		animationFrameInterval = 1;
		displayLink = nil;
		animationTimer = nil;
		
		NSString* reqSysVer = @"3.1";
		NSString* currSysVer = [[UIDevice currentDevice] systemVersion];
		if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
			displayLinkSupported = TRUE;

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
	m_application->update();
}

- (void) layoutSubviews
{
	//[renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;		
		if (animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}

- (void) startAnimation
{
	if (!animating)
	{
		if (displayLinkSupported)
		{
			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			[displayLink setFrameInterval:animationFrameInterval];
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		
		animating = TRUE;
	}
}

- (void)stopAnimation
{
	if (animating)
	{
		if (displayLinkSupported)
		{
			[displayLink invalidate];
			displayLink = nil;
		}
		else
		{
			[animationTimer invalidate];
			animationTimer = nil;
		}
		
		animating = FALSE;
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

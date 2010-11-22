#import "Launch/App/IPhone/EAGLView.h"

#include "Launch/App/Application.h"
#include "Launch/App/Configuration.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Log/Log.h"

using namespace traktor;

// @fixme
extern const traktor::CommandLine* g_cmdLine;
extern amalgam::IStateFactory* g_stateFactory;

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
/*					
		// Read configuration file.
		std::wstring configurationFileName;
		if (g_cmdLine->getCount() >= 1)
			configurationFileName = g_cmdLine->getString(0);

		if (configurationFileName.empty())
		{
			traktor::log::error << L"No configuration file" << Endl;
			return 0;
		}
		
		configurationFileName = FileSystem::getInstance().getAbsolutePath(configurationFileName).getPathName();
		traktor::log::info << L"Using configuration \"" << configurationFileName << L"\"" << Endl;

		Ref< IStream > configurationFile = FileSystem::getInstance().open(configurationFileName, File::FmRead);
		if (!configurationFile)
		{
			traktor::log::error << L"Unable to open configuration file \"" << configurationFileName << L"\"" << Endl;
			return 0;
		}

		Ref< amalgam::Configuration > configuration = xml::XmlDeserializer(configurationFile).readObject< amalgam::Configuration >();
		if (!configuration)
		{
			traktor::log::error << L"Unable to parse configuration file \"" << configurationFileName << L"\"" << Endl;
			return 0;
		}

		configurationFile->close();
*/
		Ref< amalgam::Configuration > configuration = new amalgam::Configuration();
		configuration->setDatabase(L"192.168.143.23:33666/Output-IPhone-OpenGLES2");
		configuration->setRenderSystem(L"traktor.render.RenderSystemOpenGLES2");

		// Create application.
		m_application = new amalgam::Application();
		if (!m_application->create(
			g_stateFactory,
			self,
			0,
			0,
			0,
			0,
			2,
			false,
			false,
			false,
			false,
			configuration
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

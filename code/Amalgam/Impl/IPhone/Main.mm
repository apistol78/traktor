#include <Core/Misc/CommandLine.h>
#include "Amalgam/Launch.h"

#import <UIKit/UIKit.h>
#import "Amalgam/Impl/IPhone/EAGLView.h"
#import "Amalgam/Impl/IPhone/LaunchAppDelegate.h"

namespace traktor
{
	namespace amalgam
	{

const CommandLine* g_cmdLine = 0;
std::wstring g_settingsFileName;
online::ISessionManagerProvider* g_sessionManagerProvider = 0;
IStateFactory* g_stateFactory = 0;

int32_t amalgamMain(
	const CommandLine& cmdLine,
	const std::wstring& settingsFileName,
	online::ISessionManagerProvider* sessionManagerProvider,
	IStateFactory* stateFactory
)
{
	g_cmdLine = &cmdLine;
	g_settingsFileName = settingsFileName;
	g_sessionManagerProvider = sessionManagerProvider;
	g_stateFactory = stateFactory;
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	// Ensure class is properly linked; ie not removed as unused by the linker.
	[LaunchAppDelegate description];
	[EAGLView description];
	
	int rv = UIApplicationMain(0, nil, nil, @"LaunchAppDelegate");
	[pool release];
	
	return rv;
}

	}
}

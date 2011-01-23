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
	g_sessionManagerProvider = sessionManagerProvider;
	g_stateFactory = stateFactory;
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	// Ensures that the app delegate will be linked correctly.
	[EAGLView description];
	[LaunchAppDelegate description];
	
	int rv = UIApplicationMain(0, nil, nil, nil);
	[pool release];
	
	return rv;
}

	}
}

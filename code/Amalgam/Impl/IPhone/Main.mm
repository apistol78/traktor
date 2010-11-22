#include <Core/Misc/CommandLine.h>
#include "Launch/Launch.h"

#import <UIKit/UIKit.h>
#import "Launch/App/IPhone/EAGLView.h"
#import "Launch/App/IPhone/LaunchAppDelegate.h"

const traktor::CommandLine* g_cmdLine = 0;
amalgam::IStateFactory* g_stateFactory = 0;

int32_t amalgamMain(const traktor::CommandLine& cmdLine, amalgam::IStateFactory* stateFactory)
{
	g_cmdLine = &cmdLine;
	g_stateFactory = stateFactory;
	
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	// Ensures that the app delegate will be linked correctly.
	[EAGLView description];
	[LaunchAppDelegate description];
	
	int rv = UIApplicationMain(0, nil, nil, nil);
	[pool release];
	
	return rv;
}

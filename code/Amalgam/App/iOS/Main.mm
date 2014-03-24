#import <UIKit/UIKit.h>
#import "Amalgam/App/iOS/EAGLView.h"
#import "Amalgam/App/iOS/LaunchAppDelegate.h"

using namespace traktor;

int main(int argc, const char** argv)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	// Ensure class is properly linked; ie not removed as unused by the linker.
	[LaunchAppDelegate description];
	[EAGLView description];
	
	int rv = UIApplicationMain(0, nil, nil, @"LaunchAppDelegate");
	[pool release];
	
	return rv;
}


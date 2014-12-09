#import "Amalgam/App/iOS/AppViewController.h"
#import "Amalgam/App/iOS/LaunchAppDelegate.h"

#include "Core/Log/Log.h"

using namespace traktor;

extern void applicationStart();
extern void applicationEnd();
extern void applicationSuspend();
extern void applicationResume();

@interface LaunchAppDelegate ()
{
    UIWindow* window;
	AppViewController* viewController;
}

@end

@implementation LaunchAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	// Create window.
	window = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
	[window setBackgroundColor: [UIColor whiteColor]];

	// Create view controller.
	viewController = [[AppViewController alloc] initWithNibName: nil bundle: nil];
	window.rootViewController = viewController;

	[window makeKeyAndVisible];

	// Create application and start animating.
	if ([viewController createApplication] == NO)
		return NO;

	applicationStart();

	[viewController startAnimation];
    return YES;
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	if (viewController != nil)
		[viewController stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	if (viewController != nil)
		[viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	applicationEnd();

	if (viewController != nil)
		[viewController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	applicationSuspend();
	
	if (viewController != nil)
		[viewController suspend];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	if (viewController != nil)
		[viewController resume];

	applicationResume();
}

- (void) dealloc
{
	[viewController release];
	[window release];	
	[super dealloc];
}

@end

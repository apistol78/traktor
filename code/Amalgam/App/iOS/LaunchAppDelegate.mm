#import "Amalgam/App/iOS/AppViewController.h"
#import "Amalgam/App/iOS/LaunchAppDelegate.h"

#include "Core/Log/Log.h"

using namespace traktor;

@interface LaunchAppDelegate ()
{
    UIWindow* window;
	AppViewController* viewController;
}

@end

@implementation LaunchAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	log::info << L"Creating window ..." << Endl;

	// Create window.
	window = [[UIWindow alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
	[window setBackgroundColor: [UIColor whiteColor]];

	log::info << L"Creating view controller ..." << Endl;

	// Create view controller.
	viewController = [[AppViewController alloc] init];
	window.rootViewController = viewController;
	//[viewController loadView];

	[window makeKeyAndVisible];

	log::info << L"End of didFinishLaunchingWithOptions" << Endl;
    return YES;
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	log::info << L"applicationWillResignActive" << Endl;
	if (viewController != nil)
		[viewController stopAnimation];
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	log::info << L"applicationDidBecomeActive" << Endl;
	if (viewController != nil)
		[viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	log::info << L"applicationWillTerminate" << Endl;
	if (viewController != nil)
		[viewController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	log::info << L"applicationDidEnterBackground" << Endl;
	if (viewController != nil)
		[viewController stopAnimation];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	log::info << L"applicationWillEnterForeground" << Endl;
	if (viewController != nil)
		[viewController startAnimation];
}

- (void) dealloc
{
	[viewController release];
	[window release];	
	[super dealloc];
}

@end

#import "Amalgam/App/iOS/AppViewController.h"
#import "Amalgam/App/iOS/EAGLView.h"

#include "Core/Log/Log.h"

using namespace traktor;

@interface AppViewController ()
{
	EAGLView* glView;
}

@end

@implementation AppViewController

- (id) init
{
	glView = 0;
	return self;
}

- (void) loadView
{
	log::info << L"loadView" << Endl;

	glView = [[EAGLView alloc] initWithFrame: [[UIScreen mainScreen] bounds]];

	if ([glView createApplication] != YES)
		log::error << L"Unable to create application; application need to be reinstalled." << Endl;

	self.view = glView;

	log::info << L"End of loadView" << Endl;
}

- (void) viewDidLoad
{
	log::info << L"viewDidLoad" << Endl;

	[super viewDidLoad];
	[glView startAnimation];

	log::info << L"End of viewDidLoad" << Endl;
}

- (BOOL)prefersStatusBarHidden
{
	return YES;
}

- (void) startAnimation
{
	if (glView != nil)
		[glView startAnimation];
}

- (void) stopAnimation
{
	if (glView != nil)
		[glView stopAnimation];
}

@end

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

- (void) loadView
{
	glView = [[EAGLView alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
	self.view = glView;
}

- (BOOL) prefersStatusBarHidden
{
	return YES;
}

- (BOOL) createApplication
{
	if (glView != nil)
		return [glView createApplication];
	else
		return NO;
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

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import "Runtime/App/iOS/AppViewController.h"
#import "Runtime/App/iOS/AppView.h"

#include "Core/Log/Log.h"

using namespace traktor;

@interface AppViewController ()
{
	AppView* glView;
}

@end

@implementation AppViewController

- (void) loadView
{
	glView = [[AppView alloc] initWithFrame: [[UIScreen mainScreen] bounds]];
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

- (void) suspend
{
	if (glView != nil)
		[glView suspend];
}

- (void) resume
{
	if (glView != nil)
		[glView resume];
}

@end

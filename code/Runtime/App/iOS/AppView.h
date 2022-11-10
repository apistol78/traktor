/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include "Core/Ref.h"

namespace traktor
{

class Thread;

}

@interface AppView : UIView

- (BOOL) createApplication;

- (void) mainLoop;

- (void) drawView:(id)sender;

- (void) layoutSubviews;

- (void) startAnimation;

- (void) stopAnimation;

- (void) suspend;

- (void) resume;

- (void) dealloc;

@end


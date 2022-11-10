/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef traktor_amalgam_AppViewController_H
#define traktor_amalgam_AppViewController_H

#import <UIKit/UIKit.h>

@interface AppViewController : UIViewController

- (BOOL) createApplication;

- (void) startAnimation;

- (void) stopAnimation;

- (void) suspend;

- (void) resume;

@end

#endif	// traktor_amalgam_AppViewController_H


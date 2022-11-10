/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import <Cocoa/Cocoa.h>

@interface CGLWindowDelegate : NSObject< NSWindowDelegate >
{
	BOOL m_resized;
	BOOL m_resizing;
}

- (id) init;

- (void) windowWillStartLiveResize: (NSNotification *)notification;

- (void) windowDidEndLiveResize: (NSNotification *)notification;

- (void) windowDidResize: (NSNotification *)notification;

- (BOOL) resizedSinceLast;

- (BOOL) isInLiveResize;

@end

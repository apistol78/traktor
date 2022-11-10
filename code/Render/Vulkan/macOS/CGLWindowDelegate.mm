/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import "CGLWindowDelegate.h"

@implementation CGLWindowDelegate

- (id) init
{
	m_resized = NO;
	m_resizing = NO;
	return self;
}

- (void) windowWillStartLiveResize: (NSNotification *)notification
{
	m_resizing = YES;
}

- (void) windowDidEndLiveResize: (NSNotification *)notification
{
	m_resizing = NO;
}

- (void) windowDidResize: (NSNotification *)notification
{
	m_resized = YES;
}

- (BOOL) resizedSinceLast
{
	BOOL resized = m_resized;
	m_resized = NO;
	return resized;
}

- (BOOL) isInLiveResize
{
	return m_resizing;
}

@end

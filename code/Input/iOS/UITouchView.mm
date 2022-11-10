/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import "Input/iOS/UITouchView.h"

@implementation UITouchView

- (void) setCallback: (traktor::input::ITouchViewCallback*) callback
{
	m_callback = callback;
}

- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesBegan(touches, event);
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesMoved(touches, event);
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesEnded(touches, event);
}

- (void) touchesCancelled: (NSSet*) touches withEvent: (UIEvent*) event
{
	if (m_callback)
		m_callback->touchesCancelled(touches, event);
}

@end

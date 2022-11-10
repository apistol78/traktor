/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{

struct INSControlEventsCallback
{
	virtual bool event_drawRect(const NSRect& rect) = 0;

	virtual bool event_viewDidEndLiveResize() = 0;

	virtual bool event_mouseDown(NSEvent* theEvent, int button) = 0;

	virtual bool event_mouseUp(NSEvent* theEvent, int button) = 0;

	virtual bool event_mouseMoved(NSEvent* theEvent, int button) = 0;

	virtual bool event_keyDown(NSEvent* theEvent) = 0;

	virtual bool event_keyUp(NSEvent* theEvent) = 0;

	virtual bool event_performKeyEquivalent(NSEvent* theEvent) = 0;
};

	}
}

@interface NSCustomControl : NSControl
{
	traktor::ui::INSControlEventsCallback* m_eventsCallback;
	NSString* m_string;
	NSFont* m_font;
}

- (id) initWithFrame: (NSRect)frameRect;

- (void) setCallback: (traktor::ui::INSControlEventsCallback*)eventsCallback;

- (void) setStringValue: (NSString*)aString;

- (NSString*) stringValue;

- (void) setFont: (NSFont*)font;

- (NSFont*) font;

- (BOOL) acceptsFirstResponder;

- (BOOL) acceptsFirstMouse;

- (BOOL) canBecomeKeyView;

- (BOOL) isFlipped;

- (void) drawRect: (NSRect)rect;

- (void) viewDidEndLiveResize;

- (void) mouseDown: (NSEvent*)theEvent;

- (void) mouseUp: (NSEvent*)theEvent;

- (void) rightMouseDown: (NSEvent*)theEvent;

- (void) rightMouseUp: (NSEvent*)theEvent;

- (void) mouseMoved: (NSEvent*)theEvent;

- (void) mouseDragged: (NSEvent*)theEvent;

- (void) rightMouseDragged: (NSEvent*)theEvent;

- (void) keyDown: (NSEvent*)theEvent;

- (void) keyUp: (NSEvent*)theEvent;

- (BOOL) performKeyEquivalent: (NSEvent *)theEvent;

@end


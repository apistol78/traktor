/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NSCustomControl_H
#define traktor_ui_NSCustomControl_H

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

#endif	// traktor_ui_NSCustomControl_H

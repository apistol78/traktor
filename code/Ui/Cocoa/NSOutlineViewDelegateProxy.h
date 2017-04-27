/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NSOutlineViewDelegateProxy_H
#define traktor_ui_NSOutlineViewDelegateProxy_H

#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct INSOutlineViewEventsCallback
{
	virtual void event_selectionDidChange() = 0;
	
	virtual void event_rightMouseDown(NSEvent* event) = 0;
	
	virtual void event_willDisplayCell(NSCell* cell, NSTableColumn* tableColumn, void* item) = 0;
};
	
	}
}

@interface NSOutlineViewDelegateProxy : NSObject< NSOutlineViewDelegate >
{
	traktor::ui::INSOutlineViewEventsCallback* m_eventsCallback;
}

- (id) init;

- (void) setCallback: (traktor::ui::INSOutlineViewEventsCallback*)eventsCallback;

- (void) outlineViewSelectionDidChange: (NSNotification*)notification;

- (void) outlineViewRightMouseDown: (NSEvent*)event;

- (void) outlineView: (NSOutlineView *)outlineView willDisplayCell: (id)cell forTableColumn: (NSTableColumn*)tableColumn item: (id)item;

@end

#endif	// traktor_ui_NSOutlineViewDelegateProxy_H

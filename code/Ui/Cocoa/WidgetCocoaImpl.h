/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_WidgetCocoaImpl_H
#define traktor_ui_WidgetCocoaImpl_H

#import <Cocoa/Cocoa.h>

#include <map>
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Cocoa/NSTargetProxy.h"
#include "Ui/Cocoa/NSNotificationProxy.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Events/AllEvents.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{
	
class EventSubject;

template < typename ControlType, typename NSControlType, typename NSViewType = NSControlType >	
class WidgetCocoaImpl
:	public ControlType
,	public INotificationProxyCallback
{
public:
	typedef WidgetCocoaImpl< ControlType, NSControlType, NSViewType > class_t;

	WidgetCocoaImpl(EventSubject* owner)
	:	m_owner(owner)
	,	m_control(0)
	,	m_view(0)
	,	m_notificationProxy(0)
	,	m_haveFocus(false)
	,	m_tracking(false)
	{
	}
		
	virtual void destroy() T_OVERRIDE
	{
		// Remove notification observer.
		if (m_notificationProxy)
		{
			NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
			[notificationCenter removeObserver: m_notificationProxy];
			
			[m_notificationProxy autorelease];
			m_notificationProxy = 0;
		}

		// Release all timers.
		for (std::map< int, NSTimer* >::iterator i = m_timers.begin(); i != m_timers.end(); ++i)
			[i->second invalidate];
			
		m_timers.clear();
		
		// Remove widget from parent.
		NSView* view = getView();
		if (view)
			[view removeFromSuperview];
		
		// Release objects.
		if (m_control)
		{
			[m_control autorelease];
			m_control = 0;
		}
		
		if (m_view)
		{
			[m_view autorelease];
			m_view = 0;
		}
	}

	virtual void setParent(IWidget* parent) T_OVERRIDE
	{
		NSView* view = getView();
		
		[view removeFromSuperview];
	
		NSView* contentView = (NSView*)parent->getInternalHandle();
		T_ASSERT (contentView);
		
		[contentView addSubview: view];
	}

	virtual void setText(const std::wstring& text) T_OVERRIDE
	{
		[getControl() setStringValue: makeNSString(text)];
	}

	virtual std::wstring getText() const T_OVERRIDE
	{
		return fromNSString([m_control stringValue]);
	}

	virtual void setToolTipText(const std::wstring& text) T_OVERRIDE
	{
	}

	virtual void setForeground() T_OVERRIDE
	{
	}

	virtual bool isForeground() const T_OVERRIDE
	{
		return false;
	}

	virtual void setVisible(bool visible) T_OVERRIDE
	{
		[getView() setHidden: visible ? NO : YES];
	}

	virtual bool isVisible(bool includingParents) const T_OVERRIDE
	{
		if (!includingParents)
			return [getView() isHidden] == NO;
		else
			return [getView() isHiddenOrHasHiddenAncestor] == NO;
	}

	virtual void setActive() T_OVERRIDE
	{
	}

	virtual void setEnable(bool enable) T_OVERRIDE
	{
		[getControl() setEnabled: enable ? YES : NO];
	}

	virtual bool isEnable() const T_OVERRIDE
	{
		return [getControl() isEnabled] == YES;
	}

	virtual bool hasFocus() const T_OVERRIDE
	{
		NSWindow* window = [getControl() window];
		if (!window)
			return false;
			
		for (NSResponder* responder = [window firstResponder]; responder; responder = [responder nextResponder])
		{
			if (responder == m_control)
				return true;
		}
		
		return false;
	}

	virtual bool containFocus() const T_OVERRIDE
	{
		return hasFocus();
	}

	virtual void setFocus() T_OVERRIDE
	{
		NSWindow* window = [getControl() window];
		if (window)
			[window makeFirstResponder: m_control];
	}

	virtual bool hasCapture() const T_OVERRIDE
	{
		return m_tracking;
	}

	virtual void setCapture() T_OVERRIDE
	{
		m_tracking = true;
	}

	virtual void releaseCapture() T_OVERRIDE
	{
		m_tracking = false;
	}

	virtual void startTimer(int interval, int id) T_OVERRIDE
	{
		ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< class_t >(
			this,
			&class_t::callbackTimer,
			0
		);
	
		NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
		[targetProxy setCallback: targetCallback];
		
		NSTimer* timer = [[NSTimer alloc]
			initWithFireDate: [NSDate date]
			interval: (double)interval / 1000.0
			target: targetProxy
			selector: @selector(dispatchActionCallback:)
			userInfo: nil
			repeats: YES
		];
		
		[[NSRunLoop currentRunLoop] addTimer: timer forMode: NSDefaultRunLoopMode];
		[[NSRunLoop currentRunLoop] addTimer: timer forMode: NSModalPanelRunLoopMode];
		
		m_timers[id] = timer;
	}
	
	virtual void stopTimer(int id) T_OVERRIDE
	{
		std::map< int, NSTimer* >::iterator i = m_timers.find(id);
		if (i != m_timers.end())
		{
			[i->second invalidate];
			m_timers.erase(i);
		}
	}

	virtual void setOutline(const Point* p, int np) T_OVERRIDE
	{
	}

	virtual void setRect(const Rect& rect) T_OVERRIDE
	{
		[getView() setFrame: makeNSRect(rect)];
		raiseSizeEvent();
	}

	virtual Rect getRect() const T_OVERRIDE
	{
		NSRect rc = [getView() frame];
		return fromNSRect(rc);
	}

	virtual Rect getInnerRect() const T_OVERRIDE
	{
		NSRect rc = [getView() frame];
		rc.origin.x =
		rc.origin.y = 0;
		return fromNSRect(rc);
	}

	virtual Rect getNormalRect() const T_OVERRIDE
	{
		return Rect(0, 0, 0, 0);
	}

	virtual Size getTextExtent(const std::wstring& text) const T_OVERRIDE
	{
		NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
		
		NSFont* font = [getControl() font];
		if (!font)
			font = [NSFont controlContentFontOfSize: 11];
		
		[attributes setObject: font forKey:NSFontAttributeName];

		NSString* str = makeNSString(text);
		NSSize sz = [str sizeWithAttributes: attributes];
		
		return fromNSSize(sz);
	}

	virtual void setFont(const Font& font) T_OVERRIDE
	{
		NSFont* nsfnt = [NSFont
			fontWithName: makeNSString(font.getFace())
			size: (CGFloat)font.getPixelSize()
		];
		if (!nsfnt)
			return;
		
		[getControl() setFont: nsfnt];
	}

	virtual Font getFont() const T_OVERRIDE
	{
		NSFont* font = [getControl() font];
		if (!font)
			font = [NSFont controlContentFontOfSize: 11];

		return Font(
			fromNSString([font fontName]),
			[font pointSize]
		);
	}

	virtual void setCursor(Cursor cursor) T_OVERRIDE
	{
	}

	virtual Point getMousePosition(bool relative) const T_OVERRIDE
	{
		return Point(0, 0);
	}

	virtual Point screenToClient(const Point& pt) const T_OVERRIDE
	{
		NSPoint basePt = [m_view convertPoint: makeNSPoint(pt) toView: nil];
		return fromNSPoint(basePt);
	}

	virtual Point clientToScreen(const Point& pt) const T_OVERRIDE
	{
		NSPoint localPt = [m_view convertPoint: makeNSPoint(pt) fromView: nil];
		return fromNSPoint(localPt);
	}

	virtual bool hitTest(const Point& pt) const T_OVERRIDE
	{
		return false;
	}

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects) T_OVERRIDE
	{
		for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
		{
			if (i->widget)
				i->widget->setRect(i->rect);
		}
	}

	virtual Size getMinimumSize() const T_OVERRIDE
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const T_OVERRIDE
	{
		NSSize idealSize = [[getControl() cell] cellSize];
		return Size(idealSize.width, idealSize.height);
	}

	virtual Size getMaximumSize() const T_OVERRIDE
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate) T_OVERRIDE
	{
		if (immediate)
			[getView() display];
		else
			[getView() setNeedsDisplay: YES];
	}

	virtual void* getInternalHandle() T_OVERRIDE
	{
		return m_control;
	}

	virtual SystemWindow getSystemWindow() T_OVERRIDE
	{
		return SystemWindow(m_control);
	}

protected:
	EventSubject* m_owner;
	NSControlType* m_control;
	NSViewType* m_view;
	NSNotificationProxy* m_notificationProxy;
	std::map< int, NSTimer* > m_timers;
	bool m_haveFocus;
	bool m_tracking;
	
	bool internalCreate()
	{
		m_notificationProxy = [[NSNotificationProxy alloc] init];
		[m_notificationProxy setCallback: this];

		NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
		[notificationCenter
			addObserver: m_notificationProxy
			selector: @selector(dispatchNotificationCallback:)
			name: NSWindowDidUpdateNotification
			object: [m_control window]
		];
		
		return true;
	}
	
	NSControl* getControl() const
	{
		return m_control;
	}
	
	NSView* getView() const
	{
		return m_view != 0 ? m_view : (NSView*)m_control;
	}
	
	void raiseSizeEvent()
	{
		Size sz = getRect().getSize();
		SizeEvent s(m_owner, sz);
		m_owner->raiseEvent(&s);
	}
	
	void notificationProxy_recv(NSNotification* notification)
	{
		bool haveFocus = hasFocus();
		if (m_haveFocus != haveFocus)
		{
			FocusEvent focusEvent(m_owner, haveFocus);
			m_owner->raiseEvent(&focusEvent);
			m_haveFocus = haveFocus;
		}
	}

	void callbackTimer(void* controlId)
	{
		TimerEvent timerEvent(m_owner, 0);
		m_owner->raiseEvent(&timerEvent);
	}
};
	
	}
}

#endif	// traktor_ui_WidgetCocoaImpl_H

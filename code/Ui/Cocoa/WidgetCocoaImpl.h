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

#include "Core/Log/Log.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Cocoa/NSTargetProxy.h"
#include "Ui/Cocoa/NSNotificationProxy.h"
#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

template < typename ControlType, typename NSControlType >
class WidgetCocoaImpl
:	public ControlType
,	public IFontMetric
,	public INotificationProxyCallback
{
public:
	typedef WidgetCocoaImpl< ControlType, NSControlType > class_t;

	explicit WidgetCocoaImpl(EventSubject* owner)
	:	m_owner(owner)
	{
	}

	virtual void destroy() override
	{
		// Remove notification observer.
		if (m_notificationProxy)
		{
			NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
			[notificationCenter removeObserver: m_notificationProxy];
			m_notificationProxy = nullptr;
		}

		// Release all timers.
		if (m_timer)
		{
			[m_timer invalidate];
			m_timer = nullptr;
		}

		// Remove widget from parent.
		NSView* view = m_control;
		if (view)
			[view removeFromSuperview];

		// Release objects.
		m_control = nullptr;
	}

	virtual void setParent(IWidget* parent) override
	{
		NSView* view = m_control;

		[view removeFromSuperview];

		NSView* contentView = (__bridge NSView*)parent->getInternalHandle();
		T_ASSERT (contentView);

		[contentView addSubview: view];
	}

	virtual void setText(const std::wstring& text) override
	{
		[m_control setStringValue: makeNSString(text)];
	}

	virtual std::wstring getText() const override
	{
		return fromNSString([m_control stringValue]);
	}

	virtual void setForeground() override
	{
	}

	virtual bool isForeground() const override
	{
		return false;
	}

	virtual void setVisible(bool visible) override
	{
		[m_control setHidden: visible ? NO : YES];
	}

	virtual bool isVisible() const override
	{
		return [m_control isHidden] == NO;
	}

	virtual void setEnable(bool enable) override
	{
		[m_control setEnabled: enable ? YES : NO];
	}

	virtual bool isEnable() const override
	{
		return [m_control isEnabled] == YES;
	}

	virtual bool hasFocus() const override
	{
		NSWindow* window = [m_control window];
		if (!window)
			return false;

		if ([window firstResponder] == m_control)
			return true;

		return false;
	}

	virtual void setFocus() override
	{
		NSWindow* window = [m_control window];
		if (window)
			[window makeFirstResponder: m_control];
	}

	virtual bool hasCapture() const override
	{
		return m_tracking;
	}

	virtual void setCapture() override
	{
		m_tracking = true;
	}

	virtual void releaseCapture() override
	{
		m_tracking = false;
	}

	virtual void startTimer(int interval) override
	{
		stopTimer();

		ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< class_t >(
			this,
			&class_t::callbackTimer,
			nullptr
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

		m_timer = timer;
	}

	virtual void stopTimer() override
	{
		if (m_timer)
		{
			[m_timer invalidate];
			m_timer = nullptr;
		}
	}

	virtual void setRect(const Rect& rect) override
	{
		[m_control setFrame: makeNSRect(rect)];
		raiseSizeEvent();
	}

	virtual Rect getRect() const override
	{
		NSRect rc = [m_control frame];
		return fromNSRect(rc);
	}

	virtual Rect getInnerRect() const override
	{
		NSRect rc = [m_control frame];
		rc.origin.x =
		rc.origin.y = 0;
		return fromNSRect(rc);
	}

	virtual Rect getNormalRect() const override
	{
		return Rect(0, 0, 0, 0);
	}

	virtual void setFont(const Font& font) override
	{
		NSFont* nsfnt = [NSFont
			fontWithName: makeNSString(font.getFace())
			size: (CGFloat)font.getPixelSize()
		];
		if (!nsfnt)
			return;

		[m_control setFont: nsfnt];
	}

	virtual Font getFont() const override
	{
		NSFont* font = [m_control font];
		if (!font)
		{
			font = getDefaultFont();
			if (!font)
				return Font();
		}

		return Font(
			fromNSString([font fontName]),
			[font pointSize]
		);
	}

	virtual const IFontMetric* getFontMetric() const override
	{
		return this;
	}

	virtual void setCursor(Cursor cursor) override
	{
	}

	virtual Point getMousePosition(bool relative) const override
	{
		return Point(0, 0);
	}

	virtual Point screenToClient(const Point& pt) const override
	{
		NSWindow* window = [m_control window];
		NSRect windowFrame = flipNSRect([window contentRectForFrameRect: [window frame]]);

		NSPoint pointInScreen = makeNSPoint(pt);
		NSPoint pointInWindow = pointInScreen;
		pointInWindow.x -= windowFrame.origin.x;
		pointInWindow.y -= windowFrame.origin.y;

		NSPoint clientPoint = [m_control convertPoint: pointInWindow fromView: [window contentView]];
		return fromNSPoint(clientPoint);
	}

	virtual Point clientToScreen(const Point& pt) const override
	{
		NSWindow* window = [m_control window];
		NSRect windowFrame = flipNSRect([window contentRectForFrameRect: [window frame]]);

		NSPoint pointInWindow = [[window contentView] convertPoint: makeNSPoint(pt) fromView: m_control];
		NSPoint pointInScreen = pointInWindow;
		pointInScreen.x += windowFrame.origin.x;
		pointInScreen.y += windowFrame.origin.y;

		return fromNSPoint(pointInScreen);
	}

	virtual bool hitTest(const Point& pt) const override
	{
		Point cpt = screenToClient(pt);
		Rect rcInner = getInnerRect();
		return rcInner.inside(cpt);
	}

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count) override
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			if (childRects[i].widget)
				childRects[i].widget->setRect(childRects[i].rect);
		}
	}

	virtual Size getMinimumSize() const override
	{
		return Size(0, 0);
	}

	virtual Size getPreferredSize(const Size& hint) const override
	{
		NSSize idealSize = [[m_control cell] cellSize];
		return Size(idealSize.width, idealSize.height);
	}

	virtual Size getMaximumSize() const override
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate) override
	{
		if (immediate)
			[m_control display];
		else
			[m_control setNeedsDisplay: YES];
	}

	virtual void* getInternalHandle() override
	{
		return (__bridge void*)m_control;
	}

	virtual SystemWindow getSystemWindow() override
	{
		return SystemWindow((__bridge void*)m_control);
	}

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override
	{
		NSFont* font = [m_control font];
		if (!font)
		{
			font = getDefaultFont();
			if (!font)
				return;
		}

		float a =  [font ascender];
		float b = -[font descender];
		float c =  [font leading];

		outAscent = (int32_t)a;
		outDescent = (int32_t)(b + c);
	}

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override
	{
		wchar_t s[] = { ch, 0 };
		return getExtent(s).cx;
	}

	virtual int32_t getLineSpacing() const override
	{
		return 0;
	}

	virtual Size getExtent(const std::wstring& text) const override
	{
		NSMutableDictionary* attributes = [NSMutableDictionary dictionary];

		NSFont* font = [m_control font];
		if (!font)
		{
			font = getDefaultFont();
			if (!font)
				return Size(0, 0);
		}

		[attributes setObject: font forKey:NSFontAttributeName];

		NSString* str = makeNSString(text);
		NSSize sz = [str sizeWithAttributes: attributes];

		return fromNSSize(sz);
	}

protected:
	EventSubject* m_owner = nullptr;
	NSControlType* m_control = nullptr;
	NSNotificationProxy* m_notificationProxy = nullptr;
	NSTimer* m_timer = nullptr;
	bool m_haveFocus = false;
	bool m_tracking = false;

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

	void raiseSizeEvent()
	{
		Size sz = getRect().getSize();
		SizeEvent s(m_owner, sz);
		m_owner->raiseEvent(&s);
	}

	virtual void notificationProxy_recv(NSNotification* notification) override final
	{
		bool haveFocus = hasFocus();
		if (m_haveFocus != haveFocus)
		{
			FocusEvent focusEvent(m_owner, haveFocus);
			m_owner->raiseEvent(&focusEvent);
			m_haveFocus = haveFocus;
		}
	}

	void callbackTimer(id controlId)
	{
		TimerEvent timerEvent(m_owner);
		m_owner->raiseEvent(&timerEvent);
	}
};

	}
}


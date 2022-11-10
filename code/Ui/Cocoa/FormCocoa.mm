/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#import "Ui/Cocoa/NSCustomControl.h"

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/FormCocoa.h"
#include "Ui/Cocoa/NSTargetProxy.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Events/MoveEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/TimerEvent.h"

namespace traktor
{
	namespace ui
	{

FormCocoa::FormCocoa(EventSubject* owner)
:	m_owner(owner)
{
}

bool FormCocoa::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	m_window = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(50, 50, width, height)
		styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
		backing: NSBackingStoreBuffered
		defer: YES
	];

	[m_window setAcceptsMouseMovedEvents: YES];
	[m_window setTitle:makeNSString(text)];

	NSWindowDelegateProxy* proxy = [[NSWindowDelegateProxy alloc] init];
	[proxy setCallback: this];

	[m_window setDelegate: proxy];

	NSView* contentView = [[NSCustomControl alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_window setContentView: contentView];

	return true;
}

void FormCocoa::setIcon(ISystemBitmap* icon)
{
}

void FormCocoa::maximize()
{
}

void FormCocoa::minimize()
{
}

void FormCocoa::restore()
{
}

bool FormCocoa::isMaximized() const
{
	return [m_window isZoomed] == YES;
}

bool FormCocoa::isMinimized() const
{
	return [m_window isZoomed] == NO;
}

void FormCocoa::hideProgress()
{
}

void FormCocoa::showProgress(int32_t current, int32_t total)
{
}

// IWidget implementation

void FormCocoa::destroy()
{
	// Release all timers.
	stopTimer();

	// Release objects.
	if (m_window)
	{
		[m_window setDelegate: nil];
		m_window = nullptr;
	}
}

void FormCocoa::setParent(IWidget* parent)
{
}

void FormCocoa::setText(const std::wstring& text)
{
	[m_window setTitle:makeNSString(text)];
}

std::wstring FormCocoa::getText() const
{
	return fromNSString([m_window title]);
}

void FormCocoa::setForeground()
{
}

bool FormCocoa::isForeground() const
{
	return false;
}

void FormCocoa::setVisible(bool visible)
{
	if (visible)
	{
		[m_window makeKeyAndOrderFront: nil];
		[m_window makeMainWindow];
	}
}

bool FormCocoa::isVisible() const
{
	return true;
}

void FormCocoa::setEnable(bool enable)
{
}

bool FormCocoa::isEnable() const
{
	return false;
}

bool FormCocoa::hasFocus() const
{
	return false;
}

void FormCocoa::setFocus()
{
}

bool FormCocoa::hasCapture() const
{
	return false;
}

void FormCocoa::setCapture()
{
}

void FormCocoa::releaseCapture()
{
}

void FormCocoa::startTimer(int interval)
{
	stopTimer();

	ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< FormCocoa >(
		this,
		&FormCocoa::callbackTimer,
		nullptr
	);

	NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
	[targetProxy setCallback: targetCallback];

	NSTimer* timer = [[NSTimer alloc]
		initWithFireDate: [NSDate dateWithTimeIntervalSinceNow: 1]
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

void FormCocoa::stopTimer()
{
	if (m_timer)
	{
		[m_timer invalidate];
		m_timer = nullptr;
	}
}

void FormCocoa::setRect(const Rect& rect)
{
	[m_window setFrame: makeNSRect(rect) display: YES];
}

Rect FormCocoa::getRect() const
{
	NSRect frame = [m_window frame];
	return fromNSRect(frame);
}

Rect FormCocoa::getInnerRect() const
{
	NSView* contentView = [m_window contentView];
	NSRect contentFrame = [contentView frame];
	contentFrame.size.width -= 1;
	contentFrame.size.height -= 1;
	return fromNSRect(contentFrame);
}

Rect FormCocoa::getNormalRect() const
{
	NSRect frame = [m_window frame];
	return fromNSRect(frame);
}

void FormCocoa::setFont(const Font& font)
{
}

Font FormCocoa::getFont() const
{
	return Font();
}

const IFontMetric* FormCocoa::getFontMetric() const
{
	return this;
}

void FormCocoa::setCursor(Cursor cursor)
{
}

Point FormCocoa::getMousePosition(bool relative) const
{
	return Point(0, 0);
}

Point FormCocoa::screenToClient(const Point& pt) const
{
	return Point(0, 0);
}

Point FormCocoa::clientToScreen(const Point& pt) const
{
	return Point(0, 0);
}

bool FormCocoa::hitTest(const Point& pt) const
{
	return false;
}

void FormCocoa::setChildRects(const IWidgetRect* childRects, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		if (childRects[i].widget)
			childRects[i].widget->setRect(childRects[i].rect);
	}
}

Size FormCocoa::getMinimumSize() const
{
	return Size(0, 0);
}

Size FormCocoa::getPreferredSize(const Size& hint) const
{
	return Size(0, 0);
}

Size FormCocoa::getMaximumSize() const
{
	return Size(0, 0);
}

void FormCocoa::update(const Rect* rc, bool immediate)
{
}

void* FormCocoa::getInternalHandle()
{
	return (__bridge void*)[m_window contentView];
}

SystemWindow FormCocoa::getSystemWindow()
{
	return SystemWindow((__bridge void*)m_window);
}

void FormCocoa::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	outAscent = 0;
	outDescent = 0;
}

int32_t FormCocoa::getAdvance(wchar_t ch, wchar_t next) const
{
	return 0;
}

int32_t FormCocoa::getLineSpacing() const
{
	return 0;
}

Size FormCocoa::getExtent(const std::wstring& text) const
{
	return Size(0, 0);
}

void FormCocoa::event_windowDidMove()
{
	Point pt = getRect().getTopLeft();
	MoveEvent m(m_owner, pt);
	m_owner->raiseEvent(&m);
}

void FormCocoa::event_windowDidResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, sz);
	m_owner->raiseEvent(&s);
}

void FormCocoa::callbackTimer(id controlId)
{
	TimerEvent timerEvent(m_owner);
	m_owner->raiseEvent(&timerEvent);
}

bool FormCocoa::event_windowShouldClose()
{
	CloseEvent closeEvent(m_owner);
	m_owner->raiseEvent(&closeEvent);
	return !closeEvent.consumed() || !closeEvent.cancelled();
}

void FormCocoa::event_windowDidBecomeKey()
{
}

void FormCocoa::event_windowDidResignKey()
{
}

void FormCocoa::event_windowDidBecomeMain()
{
}

void FormCocoa::event_windowDidResignMain()
{
}

	}
}

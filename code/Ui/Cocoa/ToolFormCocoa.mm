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
#include "Ui/Application.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Cocoa/ToolFormCocoa.h"
#include "Ui/Cocoa/NSTargetProxy.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

ToolFormCocoa::ToolFormCocoa(EventSubject* owner)
:	m_owner(owner)
{
}

bool ToolFormCocoa::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	uint32_t styleMask = 0;

	if (style & WsCaption)
		styleMask |= NSWindowStyleMaskTitled;
	if (style & WsCloseBox)
		styleMask |= NSWindowStyleMaskClosable;
	if (style & WsResizable)
		styleMask |= NSWindowStyleMaskResizable;
	if (style & WsMinimizeBox)
		styleMask |= NSWindowStyleMaskMiniaturizable;

	m_window = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(50, 50, width, height)
		styleMask: styleMask
		backing: NSBackingStoreBuffered
		defer: YES
	];

	[m_window setTitle:makeNSString(text)];

	NSWindowDelegateProxy* proxy = [[NSWindowDelegateProxy alloc] initWithCallback: this];
	[m_window setDelegate: proxy];

	NSView* contentView = [[NSCustomControl alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_window setContentView: contentView];

	if ((m_parent = parent) != nullptr)
	{
		NSControl* parentControl = (__bridge NSControl*)parent->getInternalHandle();
		NSWindow* parentWindow = [parentControl window];
		[parentWindow addChildWindow: m_window ordered: NSWindowAbove];
	}

	return true;
}

void ToolFormCocoa::setIcon(ISystemBitmap* icon)
{
}

DialogResult ToolFormCocoa::showModal()
{
	m_result = -1;
	[m_window makeKeyAndOrderFront: nil];

	NSModalSession session = [NSApp beginModalSessionForWindow: m_window];

	EventLoopCocoa* eventLoop = static_cast< EventLoopCocoa* >(Application::getInstance()->getEventLoop());
	eventLoop->pushModal(m_window);

  	while (m_result < 0)
    {
		if (!Application::getInstance()->process())
			break;
	}

	eventLoop->popModal();

    [NSApp endModalSession:session];

	return m_result;
}

void ToolFormCocoa::endModal(DialogResult result)
{
	m_result = result;
	[NSApp abortModal];
}

// IWidget implementation

void ToolFormCocoa::destroy()
{
	/*
	if (m_parent != nullptr)
	{
		NSControl* parentControl = (NSControl*)m_parent->getInternalHandle();
		NSWindow* parentWindow = [parentControl window];
		[parentWindow removeChildWindow: m_window];
		m_parent = nullptr;
	}
	*/

	// Release all timers.
	stopTimer();

	// Release objects.
	if (m_window)
	{
		[m_window orderOut: nil];
		[m_window setDelegate: nil];
		m_window = nullptr;
	}
}

void ToolFormCocoa::setParent(IWidget* parent)
{
	log::info << mbstows(T_FILE_LINE) << L": setParent NI" << Endl;
}

void ToolFormCocoa::setText(const std::wstring& text)
{
	[m_window setTitle:makeNSString(text)];
}

std::wstring ToolFormCocoa::getText() const
{
	return fromNSString([m_window title]);
}

void ToolFormCocoa::setForeground()
{
}

bool ToolFormCocoa::isForeground() const
{
	return false;
}

void ToolFormCocoa::setVisible(bool visible)
{
	if (visible)
		[m_window makeKeyAndOrderFront: nil];
	else
		[m_window orderOut: nil];
}

bool ToolFormCocoa::isVisible() const
{
	return (bool)[m_window isVisible];
}

void ToolFormCocoa::setEnable(bool enable)
{
}

bool ToolFormCocoa::isEnable() const
{
	return true;
}

bool ToolFormCocoa::hasFocus() const
{
	return false;
}

void ToolFormCocoa::setFocus()
{
}

bool ToolFormCocoa::hasCapture() const
{
	return false;
}

void ToolFormCocoa::setCapture()
{
}

void ToolFormCocoa::releaseCapture()
{
}

void ToolFormCocoa::startTimer(int interval)
{
	stopTimer();

	ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< ToolFormCocoa >(
		this,
		&ToolFormCocoa::callbackTimer,
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

void ToolFormCocoa::stopTimer()
{
	if (m_timer)
	{
		[m_timer invalidate];
		m_timer = nullptr;
	}
}

void ToolFormCocoa::setRect(const Rect& rect)
{
	NSRect frame = makeNSRect(rect);
	[m_window setFrame: flipNSRect(frame) display: YES];
}

Rect ToolFormCocoa::getRect() const
{
	NSRect frame = flipNSRect([m_window frame]);
	return fromNSRect(frame);
}

Rect ToolFormCocoa::getInnerRect() const
{
	NSView* contentView = [m_window contentView];
	NSRect contentFrame = [contentView frame];
	contentFrame.size.width -= 1;
	contentFrame.size.height -= 1;
	return fromNSRect(contentFrame);
}

Rect ToolFormCocoa::getNormalRect() const
{
	return Rect(0, 0, 0, 0);
}

void ToolFormCocoa::setFont(const Font& font)
{
}

Font ToolFormCocoa::getFont() const
{
	return Font();
}

const IFontMetric* ToolFormCocoa::getFontMetric() const
{
	return this;
}

void ToolFormCocoa::setCursor(Cursor cursor)
{
}

Point ToolFormCocoa::getMousePosition(bool relative) const
{
	return Point(0, 0);
}

Point ToolFormCocoa::screenToClient(const Point& pt) const
{
	NSRect windowFrame = flipNSRect([m_window contentRectForFrameRect: [m_window frame]]);

	NSPoint pointInScreen = makeNSPoint(pt);
	NSPoint pointInWindow = pointInScreen;
	pointInWindow.x -= windowFrame.origin.x;
	pointInWindow.y -= windowFrame.origin.y;

	return fromNSPoint(pointInWindow);
}

Point ToolFormCocoa::clientToScreen(const Point& pt) const
{
	NSRect windowFrame = flipNSRect([m_window contentRectForFrameRect: [m_window frame]]);

	NSPoint pointInScreen = makeNSPoint(pt);
	pointInScreen.x += windowFrame.origin.x;
	pointInScreen.y += windowFrame.origin.y;

	return fromNSPoint(pointInScreen);
}

bool ToolFormCocoa::hitTest(const Point& pt) const
{
	Point cpt = screenToClient(pt);
	Rect rcInner = getInnerRect();
	return rcInner.inside(cpt);
}

void ToolFormCocoa::setChildRects(const IWidgetRect* childRects, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		if (childRects[i].widget)
			childRects[i].widget->setRect(childRects[i].rect);
	}
}

Size ToolFormCocoa::getMinimumSize() const
{
	return Size(0, 0);
}

Size ToolFormCocoa::getPreferredSize(const Size& hint) const
{
	return Size(0, 0);
}

Size ToolFormCocoa::getMaximumSize() const
{
	return Size(65535, 65535);
}

void ToolFormCocoa::update(const Rect* rc, bool immediate)
{
}

void* ToolFormCocoa::getInternalHandle()
{
	return (__bridge void*)[m_window contentView];
}

SystemWindow ToolFormCocoa::getSystemWindow()
{
	return SystemWindow((__bridge void*)m_window);
}

void ToolFormCocoa::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	outAscent = 0;
	outDescent = 0;
}

int32_t ToolFormCocoa::getAdvance(wchar_t ch, wchar_t next) const
{
	return 0;
}

int32_t ToolFormCocoa::getLineSpacing() const
{
	return 0;
}

Size ToolFormCocoa::getExtent(const std::wstring& text) const
{
	NSFont* font = getDefaultFont();
	if (!font)
		return Size(0, 0);

	NSMutableDictionary* attributes = [NSMutableDictionary dictionary];
	[attributes setObject: font forKey:NSFontAttributeName];

	NSString* str = makeNSString(text);
	NSSize sz = [str sizeWithAttributes: attributes];

	return fromNSSize(sz);
}

void ToolFormCocoa::event_windowDidMove()
{
	Point pt = getRect().getTopLeft();
	MoveEvent m(m_owner, pt);
	m_owner->raiseEvent(&m);
}

void ToolFormCocoa::event_windowDidResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, sz);
	m_owner->raiseEvent(&s);
}

void ToolFormCocoa::callbackTimer(id controlId)
{
	TimerEvent timerEvent(m_owner);
	m_owner->raiseEvent(&timerEvent);
}

bool ToolFormCocoa::event_windowShouldClose()
{
	CloseEvent closeEvent(m_owner);
	m_owner->raiseEvent(&closeEvent);
	return !closeEvent.consumed() || !closeEvent.cancelled();
}

void ToolFormCocoa::event_windowDidBecomeKey()
{
}

void ToolFormCocoa::event_windowDidResignKey()
{
}

void ToolFormCocoa::event_windowDidBecomeMain()
{
}

void ToolFormCocoa::event_windowDidResignMain()
{
}

	}
}

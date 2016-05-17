#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/EventSubject.h"
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
		styleMask |= NSTitledWindowMask;
	if (style & WsCloseBox)
		styleMask |= NSClosableWindowMask;
	if (style & WsResizable)
		styleMask |= NSResizableWindowMask;
	if (style & WsMinimizeBox)
		styleMask |= NSMiniaturizableWindowMask;

	m_window = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(50, 50, width, height)
		styleMask: styleMask
		backing: NSBackingStoreBuffered
		defer: YES
	];

	[m_window setTitle:makeNSString(text)];
	
	NSWindowDelegateProxy* proxy = [[NSWindowDelegateProxy alloc] init];
	[proxy setCallback: this];
	
	[m_window setDelegate: proxy];
	
	return true;
}

void ToolFormCocoa::center()
{
	[m_window center];
}

// IWidget implementation

void ToolFormCocoa::destroy()
{
	// Release all timers.
	for (std::map< int, NSTimer* >::iterator i = m_timers.begin(); i != m_timers.end(); ++i)
		[i->second invalidate];
		
	m_timers.clear();

	// Release objects.
	if (m_window)
	{
		[m_window setDelegate: nil];
		[m_window autorelease]; m_window = 0;
	}
}

void ToolFormCocoa::setParent(IWidget* parent)
{
}

void ToolFormCocoa::setText(const std::wstring& text)
{
	[m_window setTitle:makeNSString(text)];
}

std::wstring ToolFormCocoa::getText() const
{
	return fromNSString([m_window title]);
}

void ToolFormCocoa::setToolTipText(const std::wstring& text)
{
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

bool ToolFormCocoa::isVisible(bool includingParents) const
{
	return false;
}

void ToolFormCocoa::setActive()
{
}

void ToolFormCocoa::setEnable(bool enable)
{
}

bool ToolFormCocoa::isEnable() const
{
	return false;
}

bool ToolFormCocoa::hasFocus() const
{
	return false;
}

bool ToolFormCocoa::containFocus() const
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

void ToolFormCocoa::startTimer(int interval, int id)
{
	ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< ToolFormCocoa >(
		this,
		&ToolFormCocoa::callbackTimer,
		0
	);

	NSTargetProxy* targetProxy = [[NSTargetProxy alloc] init];
	[targetProxy setCallback: targetCallback];

	NSTimer* timer = [[NSTimer alloc]
		initWithFireDate: nil
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

void ToolFormCocoa::stopTimer(int id)
{
	std::map< int, NSTimer* >::iterator i = m_timers.find(id);
	if (i != m_timers.end())
	{
		[i->second invalidate];
		m_timers.erase(i);
	}
}

void ToolFormCocoa::setOutline(const Point* p, int np)
{
}

void ToolFormCocoa::setRect(const Rect& rect)
{
	[m_window setFrame: makeNSRect(rect) display: YES];
}

Rect ToolFormCocoa::getRect() const
{
	NSRect frame = [m_window frame];
	return fromNSRect(frame);
}

Rect ToolFormCocoa::getInnerRect() const
{
	NSView* contentView = [m_window contentView];
	NSRect contentFrame = [contentView frame];
	return fromNSRect(contentFrame);
}	

Rect ToolFormCocoa::getNormalRect() const
{
	return Rect(0, 0, 0, 0);
}

Size ToolFormCocoa::getTextExtent(const std::wstring& text) const
{
	return Size(0, 0);
}

void ToolFormCocoa::setFont(const Font& font)
{
}

Font ToolFormCocoa::getFont() const
{
	return Font();
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
	return Point(0, 0);
}

Point ToolFormCocoa::clientToScreen(const Point& pt) const
{
	return Point(0, 0);
}

bool ToolFormCocoa::hitTest(const Point& pt) const
{
	return false;
}

void ToolFormCocoa::setChildRects(const std::vector< IWidgetRect >& childRects)
{
	for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
	{
		if (i->widget)
			i->widget->setRect(i->rect);
	}
}

Size ToolFormCocoa::getMinimumSize() const
{
	return Size(0, 0);
}

Size ToolFormCocoa::getPreferedSize() const
{
	return Size(0, 0);
}

Size ToolFormCocoa::getMaximumSize() const
{
	return Size(0, 0);
}

void ToolFormCocoa::update(const Rect* rc, bool immediate)
{
}

void* ToolFormCocoa::getInternalHandle()
{
	return [m_window contentView];
}

SystemWindow ToolFormCocoa::getSystemWindow()
{
	return SystemWindow(m_window);
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

void ToolFormCocoa::callbackTimer(void* controlId)
{
	TimerEvent timerEvent(m_owner, 0);
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

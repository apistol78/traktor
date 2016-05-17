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
		styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask
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

// IWidget implementation

void FormCocoa::destroy()
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

void FormCocoa::setToolTipText(const std::wstring& text)
{
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

bool FormCocoa::isVisible(bool includingParents) const
{
	return false;
}

void FormCocoa::setActive()
{
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

bool FormCocoa::containFocus() const
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

void FormCocoa::startTimer(int interval, int id)
{
	ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< FormCocoa >(
		this,
		&FormCocoa::callbackTimer,
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

void FormCocoa::stopTimer(int id)
{
	std::map< int, NSTimer* >::iterator i = m_timers.find(id);
	if (i != m_timers.end())
	{
		[i->second invalidate];
		m_timers.erase(i);
	}
}

void FormCocoa::setOutline(const Point* p, int np)
{
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
	return fromNSRect(contentFrame);
}	

Rect FormCocoa::getNormalRect() const
{
	NSRect frame = [m_window frame];
	return fromNSRect(frame);
}

Size FormCocoa::getTextExtent(const std::wstring& text) const
{
	return Size(text.length() * 16, 16);
}

void FormCocoa::setFont(const Font& font)
{
}

Font FormCocoa::getFont() const
{
	return Font();
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

void FormCocoa::setChildRects(const std::vector< IWidgetRect >& childRects)
{
	for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
	{
		if (i->widget)
			i->widget->setRect(i->rect);
	}
}

Size FormCocoa::getMinimumSize() const
{
	return Size(0, 0);
}

Size FormCocoa::getPreferedSize() const
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
	return [m_window contentView];
}

SystemWindow FormCocoa::getSystemWindow()
{
	return SystemWindow(m_window);
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

void FormCocoa::callbackTimer(void* controlId)
{
	TimerEvent timerEvent(m_owner, 0);
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

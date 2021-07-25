#import "Ui/Cocoa/NSCustomControl.h"

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/DialogCocoa.h"
#include "Ui/Cocoa/EventLoopCocoa.h"
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

DialogCocoa::DialogCocoa(EventSubject* owner)
:	m_owner(owner)
,	m_window(nullptr)
,	m_result(0)
,	m_timer(nullptr)
{
}

bool DialogCocoa::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	uint32_t styleMask = NSTitledWindowMask | NSClosableWindowMask;

	if (style & WsResizable)
		styleMask |= NSResizableWindowMask;

	m_window = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(50, 50, width, height)
		styleMask: styleMask
		backing: NSBackingStoreBuffered
		defer: TRUE
	];

	[m_window setTitle:makeNSString(text)];

	NSWindowDelegateProxy* proxy = [[NSWindowDelegateProxy alloc] initWithCallback: this];
	[m_window setDelegate: proxy];

	NSView* contentView = [[NSCustomControl alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_window setContentView: contentView];

	return true;
}

void DialogCocoa::setIcon(ISystemBitmap* icon)
{
}

int DialogCocoa::showModal()
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

void DialogCocoa::endModal(int result)
{
	m_result = result;
	[NSApp abortModal];
}

void DialogCocoa::setMinSize(const Size& minSize)
{
}

// IWidget implementation

void DialogCocoa::destroy()
{
	// Release all timers.
	stopTimer();

	// Release objects.
	if (m_window)
	{
		[m_window orderOut: nil];
		[m_window setDelegate: nil];
		[m_window release];
		m_window = nullptr;
	}
}

void DialogCocoa::setParent(IWidget* parent)
{
}

void DialogCocoa::setText(const std::wstring& text)
{
	[m_window setTitle:makeNSString(text)];
}

std::wstring DialogCocoa::getText() const
{
	return fromNSString([m_window title]);
}

void DialogCocoa::setForeground()
{
}

bool DialogCocoa::isForeground() const
{
	return false;
}

void DialogCocoa::setVisible(bool visible)
{
	if (visible)
		[m_window makeKeyAndOrderFront: nil];
}

bool DialogCocoa::isVisible() const
{
	return false;
}

void DialogCocoa::setEnable(bool enable)
{
}

bool DialogCocoa::isEnable() const
{
	return false;
}

bool DialogCocoa::hasFocus() const
{
	return false;
}

void DialogCocoa::setFocus()
{
}

bool DialogCocoa::hasCapture() const
{
	return false;
}

void DialogCocoa::setCapture()
{
}

void DialogCocoa::releaseCapture()
{
}

void DialogCocoa::startTimer(int interval)
{
	stopTimer();

	ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< DialogCocoa >(
		this,
		&DialogCocoa::callbackTimer,
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

	m_timer = timer;
}

void DialogCocoa::stopTimer()
{
	if (m_timer)
	{
		[m_timer invalidate];
		m_timer = nullptr;
	}
}

void DialogCocoa::setRect(const Rect& rect)
{
	[m_window setFrame: makeNSRect(rect) display: YES];
}

Rect DialogCocoa::getRect() const
{
	NSView* contentView = [m_window contentView];
	NSRect frame = [m_window frame];
	return fromNSRect(frame);
}

Rect DialogCocoa::getInnerRect() const
{
	NSView* contentView = [m_window contentView];
	NSRect contentFrame = [contentView frame];
	contentFrame.size.width -= 1;
	contentFrame.size.height -= 1;
	return fromNSRect(contentFrame);
}

Rect DialogCocoa::getNormalRect() const
{
	return Rect(0, 0, 0, 0);
}

void DialogCocoa::setFont(const Font& font)
{
}

Font DialogCocoa::getFont() const
{
	return Font();
}

const IFontMetric* DialogCocoa::getFontMetric() const
{
	return this;
}

void DialogCocoa::setCursor(Cursor cursor)
{
}

Point DialogCocoa::getMousePosition(bool relative) const
{
	return Point(0, 0);
}

Point DialogCocoa::screenToClient(const Point& pt) const
{
	return Point(0, 0);
}

Point DialogCocoa::clientToScreen(const Point& pt) const
{
	return Point(0, 0);
}

bool DialogCocoa::hitTest(const Point& pt) const
{
	return false;
}

void DialogCocoa::setChildRects(const IWidgetRect* childRects, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		if (childRects[i].widget)
			childRects[i].widget->setRect(childRects[i].rect);
	}
}

Size DialogCocoa::getMinimumSize() const
{
	return Size(0, 0);
}

Size DialogCocoa::getPreferedSize() const
{
	return Size(0, 0);
}

Size DialogCocoa::getMaximumSize() const
{
	return Size(0, 0);
}

void DialogCocoa::update(const Rect* rc, bool immediate)
{
}

void* DialogCocoa::getInternalHandle()
{
	return [m_window contentView];
}

SystemWindow DialogCocoa::getSystemWindow()
{
	return SystemWindow(m_window);
}

void DialogCocoa::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	outAscent = 0;
	outDescent = 0;
}

int32_t DialogCocoa::getAdvance(wchar_t ch, wchar_t next) const
{
	return 0;
}

int32_t DialogCocoa::getLineSpacing() const
{
	return 0;
}

Size DialogCocoa::getExtent(const std::wstring& text) const
{
	return Size(0, 0);
}

void DialogCocoa::event_windowDidMove()
{
	Point pt = getRect().getTopLeft();
	MoveEvent m(m_owner, pt);
	m_owner->raiseEvent(&m);
}

void DialogCocoa::event_windowDidResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, sz);
	m_owner->raiseEvent(&s);
}

bool DialogCocoa::event_windowShouldClose()
{
	CloseEvent c(m_owner);
	m_owner->raiseEvent(&c);
	if (!c.consumed() || !c.cancelled())
	{
		if ([NSApp modalWindow] == m_window)
			[NSApp stopModal];

		m_result = DrCancel;
		return true;
	}
	else
		return false;
}

void DialogCocoa::callbackTimer(void* controlId)
{
	TimerEvent timerEvent(m_owner);
	m_owner->raiseEvent(&timerEvent);
}

void DialogCocoa::event_windowDidBecomeKey()
{
}

void DialogCocoa::event_windowDidResignKey()
{
}

void DialogCocoa::event_windowDidBecomeMain()
{
}

void DialogCocoa::event_windowDidResignMain()
{
}

	}
}

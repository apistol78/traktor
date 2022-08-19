#import "Ui/Cocoa/NSCustomControl.h"

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/CanvasCocoa.h"
#include "Ui/Cocoa/DialogCocoa.h"
#include "Ui/Cocoa/EventLoopCocoa.h"
#include "Ui/Cocoa/NSTargetProxy.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace ui
	{

DialogCocoa::DialogCocoa(EventSubject* owner)
:	m_owner(owner)
,	m_window(nullptr)
,	m_control(nullptr)
,	m_result(DialogResult::Ok)
,	m_timer(nullptr)
{
}

bool DialogCocoa::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	uint32_t styleMask = 0;

	if ((style & (WsSystemBox | WsCaption)) != 0)
		styleMask |= NSWindowStyleMaskTitled;

	if ((style & WsMinimizeBox) != 0)
		styleMask |= NSWindowStyleMaskMiniaturizable;

	if ((style & WsCloseBox) != 0)
		styleMask |= NSWindowStyleMaskClosable;

	if ((style & WsResizable) != 0)
		styleMask |= NSWindowStyleMaskResizable;

	NSRect frame = [[NSScreen mainScreen] frame];

	int32_t x = (NSWidth(frame) - width) / 2;
	int32_t y = (NSHeight(frame) - height) / 2;
	
	m_window = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(x, y, width, height)
		styleMask: styleMask
		backing: NSBackingStoreBuffered
		defer: TRUE
	];

	[m_window setTitle:makeNSString(text)];

	NSWindowDelegateProxy* proxy = [[NSWindowDelegateProxy alloc] initWithCallback: this];
	[m_window setDelegate: proxy];

	m_control = [[NSCustomControl alloc] initWithFrame: NSMakeRect(0, 0, width, height)];
	[m_control setCallback: this];

	NSFont* font = getDefaultFont();
	if (!font)
		return false;

	[m_control setFont: font];
	
	[m_window setContentView: m_control];
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

void DialogCocoa::endModal(DialogResult result)
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

	// Detach ourself from being callback.
	if (m_control)
		[m_control setCallback: nil];

	// Release objects.
	if (m_window)
	{
		[m_window orderOut: nil];
		[m_window setDelegate: nil];
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
	else
		[m_window orderOut: nil];
}

bool DialogCocoa::isVisible() const
{
	return (bool)[m_window isVisible];
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
	NSRect frame = makeNSRect(rect);
	[m_window setFrame: flipNSRect(frame) display: YES];
}

Rect DialogCocoa::getRect() const
{
	NSRect frame = flipNSRect([m_window frame]);
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
	NSRect windowFrame = flipNSRect([m_window contentRectForFrameRect: [m_window frame]]);

	NSPoint pointInScreen = makeNSPoint(pt);
	NSPoint pointInWindow = pointInScreen;
	pointInWindow.x -= windowFrame.origin.x;
	pointInWindow.y -= windowFrame.origin.y;

	return fromNSPoint(pointInWindow);
}

Point DialogCocoa::clientToScreen(const Point& pt) const
{
	NSRect windowFrame = flipNSRect([m_window contentRectForFrameRect: [m_window frame]]);

	NSPoint pointInScreen = makeNSPoint(pt);
	pointInScreen.x += windowFrame.origin.x;
	pointInScreen.y += windowFrame.origin.y;

	return fromNSPoint(pointInScreen);
}

bool DialogCocoa::hitTest(const Point& pt) const
{
	Point cpt = screenToClient(pt);
	Rect rcInner = getInnerRect();
	return rcInner.inside(cpt);
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

Size DialogCocoa::getPreferredSize(const Size& hint) const
{
	return Size(0, 0);
}

Size DialogCocoa::getMaximumSize() const
{
	return Size(65535, 65535);
}

void DialogCocoa::update(const Rect* rc, bool immediate)
{
}

void* DialogCocoa::getInternalHandle()
{
	return (__bridge void*)[m_window contentView];
}

SystemWindow DialogCocoa::getSystemWindow()
{
	return SystemWindow((__bridge void*)m_window);
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

		m_result = DialogResult::Cancel;
		return true;
	}
	else
		return false;
}

void DialogCocoa::callbackTimer(id controlId)
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

bool DialogCocoa::event_drawRect(const NSRect& rect)
{
	if (!m_owner->hasEventHandler< PaintEvent >())
		return false;

	NSFont* font = [m_control font];
	if (!font)
		return false;

	// Create wrapped canvas.
	CanvasCocoa canvasImpl(font);
	Canvas canvas(&canvasImpl);

	Rect rc = fromNSRect(rect);
	PaintEvent paintEvent(m_owner, canvas, rc);
	m_owner->raiseEvent(&paintEvent);

	return paintEvent.consumed();
}

bool DialogCocoa::event_viewDidEndLiveResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, sz);
	m_owner->raiseEvent(&s);
	return true;
}

bool DialogCocoa::event_mouseDown(NSEvent* theEvent, int button)
{
	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPoint: mousePosition fromView: nil];

	if (button == 1)
		button = MbtLeft;
	else if (button == 2)
		button = MbtRight;

	if ([theEvent clickCount] <= 1)
	{
		if (!m_owner->hasEventHandler< MouseButtonDownEvent >())
			return false;

		MouseButtonDownEvent mouseEvent(
			m_owner,
			button,
			fromNSPoint(mousePosition)
		);
		m_owner->raiseEvent(&mouseEvent);
	}
	else
	{
		if (!m_owner->hasEventHandler< MouseDoubleClickEvent >())
			return false;

		MouseDoubleClickEvent mouseEvent(
			m_owner,
			button,
			fromNSPoint(mousePosition)
		);
		m_owner->raiseEvent(&mouseEvent);
	}

	return true;
}

bool DialogCocoa::event_mouseUp(NSEvent* theEvent, int button)
{
	if (!m_owner->hasEventHandler< MouseButtonUpEvent >())
		return false;

	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPoint: mousePosition fromView: nil];

	if (button == 1)
		button = MbtLeft;
	else if (button == 2)
		button = MbtRight;

	MouseButtonUpEvent mouseEvent(
		m_owner,
		button,
		fromNSPoint(mousePosition)
	);

	if ([theEvent clickCount] <= 1)
		m_owner->raiseEvent(&mouseEvent);

	return true;
}

bool DialogCocoa::event_mouseMoved(NSEvent* theEvent, int button)
{
	if (!m_owner->hasEventHandler< MouseMoveEvent >())
		return false;

	NSPoint mousePosition = [theEvent locationInWindow];
	mousePosition = [m_control convertPoint: mousePosition fromView: nil];

	//mousePosition = [m_control convertPointFromBacking: mousePosition];
	//mousePosition.y = height + mousePosition.y;

	if (button == 1)
		button = MbtLeft;
	else if (button == 2)
		button = MbtRight;

	MouseMoveEvent mouseEvent(
		m_owner,
		button,
		fromNSPoint(mousePosition)
	);
	m_owner->raiseEvent(&mouseEvent);

	return true;
}

bool DialogCocoa::event_keyDown(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler< KeyDownEvent >())
		return false;

	NSString* chs = [theEvent characters];

	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = [chs length] > 0 ? (wchar_t)[chs characterAtIndex: 0] : 0;

	KeyDownEvent keyEvent(
		m_owner,
		translateKeyCode(keyCode),
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(&keyEvent);

	return true;
}

bool DialogCocoa::event_keyUp(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler< KeyUpEvent >())
		return false;

	NSString* chs = [theEvent characters];

	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = [chs length] > 0 ? (wchar_t)[chs characterAtIndex: 0] : 0;

	KeyUpEvent keyEvent(
		m_owner,
		translateKeyCode(keyCode),
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(&keyEvent);

	return true;
}

bool DialogCocoa::event_performKeyEquivalent(NSEvent* theEvent)
{
	if (!m_owner->hasEventHandler< KeyEvent >())
		return false;

	NSString* chs = [theEvent characters];
	if (!chs || [chs length] <= 0)
		return false;

	uint32_t keyCode = [theEvent keyCode];
	wchar_t keyChar = (wchar_t)[chs characterAtIndex: 0];

	KeyEvent keyEvent(
		m_owner,
		translateKeyCode(keyCode),
		keyCode,
		keyChar
	);
	m_owner->raiseEvent(&keyEvent);

	return true;
}

	}
}

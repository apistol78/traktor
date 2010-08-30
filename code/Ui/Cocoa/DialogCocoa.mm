#import "Ui/Cocoa/NSCustomControl.h"

#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/EventSubject.h"
#include "Ui/Cocoa/DialogCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Events/MoveEvent.h"
#include "Ui/Events/SizeEvent.h"

namespace traktor
{
	namespace ui
	{

DialogCocoa::DialogCocoa(EventSubject* owner)
:	m_owner(owner)
,	m_window(0)
,	m_result(0)
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
	
	NSWindowDelegateProxy* proxy = [[NSWindowDelegateProxy alloc] init];
	[proxy setCallback: this];
	
	[m_window setDelegate: proxy];

	NSView* contentView = [[NSCustomControl alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];
	[m_window setContentView: contentView];
	
	return true;
}

void DialogCocoa::setIcon(drawing::Image* icon)
{
}

int DialogCocoa::showModal()
{
	[m_window makeKeyAndOrderFront: nil];
	[m_window center];
	[NSApp runModalForWindow: m_window];
	[m_window orderOut: nil];
	return m_result;
}

void DialogCocoa::endModal(int result)
{
	m_result = result;
	[NSApp stopModal];
}

void DialogCocoa::setMinSize(const Size& minSize)
{
}

// IWidget implementation

void DialogCocoa::destroy()
{
	if (m_window)
	{
		[m_window setDelegate: nil];
		[m_window autorelease]; m_window = 0;
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

void DialogCocoa::setToolTipText(const std::wstring& text)
{
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

bool DialogCocoa::isVisible(bool includingParents) const
{
	return false;
}

void DialogCocoa::setActive()
{
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

bool DialogCocoa::containFocus() const
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

void DialogCocoa::startTimer(int interval, int id)
{
}

void DialogCocoa::stopTimer(int id)
{
}

void DialogCocoa::setOutline(const Point* p, int np)
{
}

void DialogCocoa::setRect(const Rect& rect)
{
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

Size DialogCocoa::getTextExtent(const std::wstring& text) const
{
	return Size(text.length() * 16, 16);
}

void DialogCocoa::setFont(const Font& font)
{
}

Font DialogCocoa::getFont() const
{
	return Font();
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

void DialogCocoa::setChildRects(const std::vector< IWidgetRect >& childRects)
{
	for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
	{
		if (i->widget)
			i->widget->setRect(i->rect);
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

void* DialogCocoa::getSystemHandle()
{
	return m_window;
}

void DialogCocoa::event_windowDidMove()
{
	Point pt = getRect().getTopLeft();
	MoveEvent m(m_owner, 0, pt);
	m_owner->raiseEvent(EiMove, &m);
}

void DialogCocoa::event_windowDidResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, 0, sz);
	m_owner->raiseEvent(EiSize, &s);
}

bool DialogCocoa::event_windowShouldClose()
{
	CloseEvent c(m_owner, 0);
	m_owner->raiseEvent(EiClose, &c);
	if (!c.consumed() || !c.cancelled())
	{
		m_result = DrCancel;
		return true;
	}
	else
		return false;
}

	}
}

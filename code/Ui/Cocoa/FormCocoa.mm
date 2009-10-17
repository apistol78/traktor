#import "Ui/Cocoa/NSCustomControl.h"

#include "Ui/Cocoa/FormCocoa.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Events/MoveEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/EventSubject.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

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

void FormCocoa::setIcon(IBitmap* icon)
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
}

void FormCocoa::setParent(IWidget* parent)
{
}

void FormCocoa::setText(const std::wstring& text)
{
}

std::wstring FormCocoa::getText() const
{
	return L"";
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
		[m_window makeKeyAndOrderFront: nil];
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
}

void FormCocoa::stopTimer(int id)
{
}

void FormCocoa::setOutline(const Point* p, int np)
{
}

void FormCocoa::setRect(const Rect& rect)
{
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
	return Rect(0, 0, 0, 0);
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

void* FormCocoa::getSystemHandle()
{
	return m_window;
}

void FormCocoa::event_windowDidMove()
{
	Point pt = getRect().getTopLeft();
	MoveEvent m(m_owner, 0, pt);
	m_owner->raiseEvent(EiMove, &m);
}

void FormCocoa::event_windowDidResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, 0, sz);
	m_owner->raiseEvent(EiSize, &s);
}

	}
}

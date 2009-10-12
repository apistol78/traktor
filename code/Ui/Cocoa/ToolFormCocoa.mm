#include "Ui/Cocoa/ToolFormCocoa.h"
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

ToolFormCocoa::ToolFormCocoa(EventSubject* owner)
:	m_owner(owner)
{
}

bool ToolFormCocoa::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	m_window = [[NSWindow alloc]
		initWithContentRect: NSMakeRect(50, 50, width, height)
		styleMask: NSTitledWindowMask | NSClosableWindowMask
		backing: NSBackingStoreBuffered
		defer: TRUE
	];

	[m_window setTitle:makeNSString(text)];
	
	NSWindowDelegateProxy* proxy = [[NSWindowDelegateProxy alloc] init];
	[proxy setCallback: this];
	
	[m_window setDelegate: proxy];
	
	return true;
}

void ToolFormCocoa::center()
{
}

// IWidget implementation

void ToolFormCocoa::destroy()
{
}

void ToolFormCocoa::setParent(IWidget* parent)
{
}

void ToolFormCocoa::setText(const std::wstring& text)
{
}

std::wstring ToolFormCocoa::getText() const
{
	return L"";
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
}

void ToolFormCocoa::stopTimer(int id)
{
}

void ToolFormCocoa::setOutline(const Point* p, int np)
{
}

void ToolFormCocoa::setRect(const Rect& rect)
{
}

Rect ToolFormCocoa::getRect() const
{
	NSRect frame = [m_window frame];
	return fromNSRect(frame);
}

Rect ToolFormCocoa::getInnerRect() const
{
	NSView* contentView = [m_window contentView];
	NSRect contentBounds = [contentView bounds];
	return fromNSRect(contentBounds);
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

void* ToolFormCocoa::getSystemHandle()
{
	return m_window;
}

void ToolFormCocoa::event_windowDidMove()
{
	Point pt = getRect().getTopLeft();
	MoveEvent m(m_owner, 0, pt);
	m_owner->raiseEvent(EiMove, &m);
	log::info << L"ToolFormCocoa::event_windowDidMove" << Endl;
}

void ToolFormCocoa::event_windowDidResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, 0, sz);
	m_owner->raiseEvent(EiSize, &s);
	log::info << L"ToolFormCocoa::event_windowDidResize" << Endl;
}

	}
}

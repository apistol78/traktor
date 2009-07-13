#include "Ui/Wx/ScrollBarWx.h"
#include "Ui/ScrollBar.h"
#include "Ui/Events/ScrollEvent.h"

namespace traktor
{
	namespace ui
	{

ScrollBarWx::ScrollBarWx(EventSubject* owner)
:	WidgetWxImpl< IScrollBar, wxScrollBar >(owner)
{
}

bool ScrollBarWx::create(IWidget* parent, int style)
{
	if (!parent)
		return false;

	int wxStyle = wxSB_HORIZONTAL;
	if (style & ScrollBar::WsVertical)
		wxStyle = wxSB_VERTICAL;

	m_window = new wxScrollBar();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wxDefaultPosition,
		wxDefaultSize,
		wxStyle
	))
	{
		m_window->Destroy();
		return false;
	}

	if (!WidgetWxImpl< IScrollBar, wxScrollBar >::create(style))
		return false;

	T_CONNECT(m_window, wxEVT_SCROLL_LINEUP,     wxScrollEvent, ScrollBarWx, &ScrollBarWx::onScroll);
	T_CONNECT(m_window, wxEVT_SCROLL_LINEDOWN,   wxScrollEvent, ScrollBarWx, &ScrollBarWx::onScroll);
	T_CONNECT(m_window, wxEVT_SCROLL_PAGEUP,     wxScrollEvent, ScrollBarWx, &ScrollBarWx::onScroll);
	T_CONNECT(m_window, wxEVT_SCROLL_PAGEDOWN,   wxScrollEvent, ScrollBarWx, &ScrollBarWx::onScroll);
	T_CONNECT(m_window, wxEVT_SCROLL_THUMBTRACK, wxScrollEvent, ScrollBarWx, &ScrollBarWx::onScroll);
	
	return true;
}

void ScrollBarWx::setRange(int range)
{
	int thumbSize = std::max(range / 10, 30);
	m_window->SetScrollbar(
		m_window->GetThumbPosition(),
		thumbSize,
		range + thumbSize,
		thumbSize
	);
}

int ScrollBarWx::getRange() const
{
	return m_window->GetRange();
}

void ScrollBarWx::setPage(int page)
{
}

int ScrollBarWx::getPage() const
{
	return 0;
}

void ScrollBarWx::setPosition(int position)
{
	m_window->SetThumbPosition(position);
}

int ScrollBarWx::getPosition() const
{
	return m_window->GetThumbPosition();
}

void ScrollBarWx::onScroll(wxScrollEvent& event)
{
	ScrollEvent scrollEvent(m_owner, 0, event.GetPosition());
	m_owner->raiseEvent(EiScroll, &scrollEvent);
}

	}
}

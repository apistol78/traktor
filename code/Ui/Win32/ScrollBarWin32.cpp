#include "Core/Log/Log.h"
#include "Ui/ScrollBar.h"
#include "Ui/Win32/ScrollBarWin32.h"

namespace traktor
{
	namespace ui
	{

ScrollBarWin32::ScrollBarWin32(EventSubject* owner) :
	WidgetWin32Impl< IScrollBar >(owner)
{
}

bool ScrollBarWin32::create(IWidget* parent, int style)
{
	DWORD dwStyle = (style & ScrollBar::WsVertical) ? SBS_VERT : SBS_HORZ;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("SCROLLBAR"),
		_T(""),
		WS_VISIBLE | WS_CHILD | dwStyle,
		0,
		0,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_REFLECTED_HSCROLL, new MethodMessageHandler< ScrollBarWin32 >(this, &ScrollBarWin32::eventScroll));
	m_hWnd.registerMessageHandler(WM_REFLECTED_VSCROLL, new MethodMessageHandler< ScrollBarWin32 >(this, &ScrollBarWin32::eventScroll));

	return true;
}

void ScrollBarWin32::setRange(int range)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;
	si.nMin = 0;
	si.nMax = range;
	SetScrollInfo(m_hWnd, SB_CTL, &si, TRUE);
}

int ScrollBarWin32::getRange() const
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;
	GetScrollInfo(m_hWnd, SB_CTL, &si);
	return int(si.nMax - si.nMin);
}

void ScrollBarWin32::setPage(int page)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE;
	si.nPage = page;
	SetScrollInfo(m_hWnd, SB_CTL, &si, TRUE);
}

int ScrollBarWin32::getPage() const
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE;
	GetScrollInfo(m_hWnd, SB_CTL, &si);
	return int(si.nPage);
}

void ScrollBarWin32::setPosition(int position)
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	si.nPos = position;
	SetScrollInfo(m_hWnd, SB_CTL, &si, TRUE);
}

int ScrollBarWin32::getPosition() const
{
	SCROLLINFO si;
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(m_hWnd, SB_CTL, &si);
	return int(si.nPos);
}

Size ScrollBarWin32::getPreferedSize() const
{
	int32_t dpi = m_hWnd.getSystemDPI();
	return Size((16 * dpi) / 96, (16 * dpi) / 96);
}

LRESULT ScrollBarWin32::eventScroll(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	SCROLLINFO si;

	std::memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	GetScrollInfo(m_hWnd, SB_CTL, &si);
	
	switch (LOWORD(wParam))
	{
	case SB_TOP:
		si.nPos = si.nMin;
		break;

	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;

	case SB_LINEDOWN:
	case SB_PAGEDOWN:		
		if (si.nPos < si.nMax)
		{
			si.nPos += (LOWORD(wParam) == SB_LINEDOWN) ? 1 : si.nPage;
			if (si.nPos > si.nMax)
				si.nPos = si.nMax;
		}
		break;

	case SB_LINEUP:
	case SB_PAGEUP:
		if (si.nPos > si.nMin)
		{
			si.nPos -= (LOWORD(wParam) == SB_LINEUP) ? 1 : si.nPage;
			if (si.nPos < si.nMin)
				si.nPos = si.nMin;
		}
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		si.nPos = HIWORD(wParam);
		break;

	case SB_ENDSCROLL:
		break;
	}

	SetScrollPos(m_hWnd, SB_CTL, si.nPos, TRUE);

	ScrollEvent scrollEvent(m_owner, si.nPos);
	m_owner->raiseEvent(&scrollEvent);

	pass = false;
	return 0;
}

	}
}

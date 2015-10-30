#include "Ui/Win32/ContainerWin32.h"

namespace traktor
{
	namespace ui
	{

ContainerWin32::ContainerWin32(EventSubject* owner)
:	WidgetWin32Impl< IContainer >(owner)
{
}

bool ContainerWin32::create(IWidget* parent, int style)
{
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	DWORD dwStyleEx = WS_EX_CONTROLPARENT;

	if (style & WsBorder)
		dwStyle |= WS_BORDER;
	if (style & WsClientBorder)
		dwStyleEx |= WS_EX_CLIENTEDGE;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("TraktorWin32Class"),
		_T(""),
		dwStyle,
		dwStyleEx,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	return true;
}

	}
}

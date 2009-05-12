#include "Ui/Win32/StaticWin32.h"

namespace traktor
{
	namespace ui
	{

StaticWin32::StaticWin32(EventSubject* owner) :
	WidgetWin32Impl< IStatic >(owner)
{
}

bool StaticWin32::create(IWidget* parent, const std::wstring& text)
{
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("STATIC"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | SS_LEFT | SS_NOTIFY,
		0,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(0))
		return false;

	return true;
}

Size StaticWin32::getPreferedSize() const
{
	return getTextExtent(getText());
}

	}
}

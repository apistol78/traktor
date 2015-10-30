#include "Ui/Button.h"
#include "Ui/Win32/ButtonWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

UINT s_buttonId = 1000;

		}

ButtonWin32::ButtonWin32(EventSubject* owner)
:	WidgetWin32Impl< IButton >(owner)
{
}

bool ButtonWin32::create(IWidget* parent, const std::wstring& text, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (style & Button::WsDefaultButton)
		nativeStyle |= BS_DEFPUSHBUTTON;
	else
		nativeStyle |= BS_PUSHBUTTON;

	if (style & Button::WsToggle)
		nativeStyle |= BS_AUTOCHECKBOX | BS_PUSHLIKE;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("BUTTON"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		s_buttonId,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(0))
		return false;

	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< ButtonWin32 >(this, &ButtonWin32::eventCommand));

	if (style & Button::WsDefaultButton)
	{
		SendMessage(
			(HWND)parent->getInternalHandle(),
			DM_SETDEFID,
			s_buttonId,
			0
		);
	}

	s_buttonId++;

	return true;
}

void ButtonWin32::setState(bool state)
{
	m_hWnd.sendMessage(BM_SETCHECK, state ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool ButtonWin32::getState() const
{
	return bool(m_hWnd.sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED);
}

Size ButtonWin32::getPreferedSize() const
{
	Size extent = getTextExtent(getText());
	SIZE ideal;
	ideal.cx = max(extent.cx + 24, 60);
	ideal.cy = extent.cy + 6;
	return Size(ideal.cx, ideal.cy);
}

LRESULT ButtonWin32::eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
	return TRUE;
}

	}
}

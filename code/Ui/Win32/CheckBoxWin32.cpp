#include "Ui/Events/ButtonClickEvent.h"
#include "Ui/Events/ButtonClickEvent.h"
#include "Ui/Win32/CheckBoxWin32.h"

namespace traktor
{
	namespace ui
	{

CheckBoxWin32::CheckBoxWin32(EventSubject* owner)
:	WidgetWin32Impl< ICheckBox >(owner)
{
}

bool CheckBoxWin32::create(IWidget* parent, const std::wstring& text, bool checked)
{
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("BUTTON"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX,
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

	m_hWnd.sendMessage(BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);

	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< CheckBoxWin32 >(this, &CheckBoxWin32::eventCommand));

	return true;
}

void CheckBoxWin32::setChecked(bool checked)
{
	m_hWnd.sendMessage(BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

bool CheckBoxWin32::isChecked() const
{
	return bool(m_hWnd.sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED);
}

Size CheckBoxWin32::getPreferedSize() const
{
	return Size(getTextExtent(getText()).cx + 16, 16);
}

LRESULT CheckBoxWin32::eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
	skip = false;
	return 0;
}

	}
}

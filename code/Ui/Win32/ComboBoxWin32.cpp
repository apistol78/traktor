/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/SelectionChangeEvent.h"
#include "Ui/Win32/ComboBoxWin32.h"

namespace traktor
{
	namespace ui
	{

ComboBoxWin32::ComboBoxWin32(EventSubject* owner)
:	WidgetWin32Impl< IComboBox >(owner)
{
}

bool ComboBoxWin32::create(IWidget* parent, const std::wstring& text, int style)
{
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("COMBOBOX"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWN,
		0,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	return true;
}

int ComboBoxWin32::add(const std::wstring& item)
{
	tstring tmp = wstots(item);
	return int(m_hWnd.sendMessage(CB_ADDSTRING, 0, (LPARAM)tmp.c_str()));
}

bool ComboBoxWin32::remove(int index)
{
	m_hWnd.sendMessage(CB_DELETESTRING, (WPARAM)index, 0);
	return true;
}

void ComboBoxWin32::removeAll()
{
	m_hWnd.sendMessage(CB_RESETCONTENT, 0, 0);
}

int ComboBoxWin32::count() const
{
	return int(m_hWnd.sendMessage(CB_GETCOUNT, 0, 0));
}

std::wstring ComboBoxWin32::get(int index) const
{
	TCHAR tmp[256];
	m_hWnd.sendMessage(CB_GETLBTEXT, (WPARAM)index, (LPARAM)tmp);
	return tstows(tmp);
}

void ComboBoxWin32::select(int index)
{
	m_hWnd.sendMessage(CB_SETCURSEL, (WPARAM)index, 0);
}

int ComboBoxWin32::getSelected() const
{
	LRESULT index = m_hWnd.sendMessage(CB_GETCURSEL, 0, 0);
	return (index != CB_ERR) ? int(index) : -1;
}

void ComboBoxWin32::setRect(const Rect& rect)
{
	Rect expand(rect);
	expand.bottom = expand.top + 8 * 16;
	WidgetWin32Impl::setRect(expand);
}

Size ComboBoxWin32::getPreferedSize() const
{
	int32_t dpi = m_hWnd.getSystemDPI();
	return Size(
		(128 * dpi) / 96,
		(24 * dpi) / 96
	);
}

LRESULT ComboBoxWin32::eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	if (HIWORD(wParam) == CBN_SELCHANGE)
	{
		SelectionChangeEvent selectionChangeEvent(m_owner);
		m_owner->raiseEvent(&selectionChangeEvent);
		pass = false;
	}
	return 0;
}

	}
}

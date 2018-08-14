/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/SelectionChangeEvent.h"
#include "Ui/Win32/DropDownWin32.h"

namespace traktor
{
	namespace ui
	{

DropDownWin32::DropDownWin32(EventSubject* owner)
:	WidgetWin32Impl< IDropDown >(owner)
{
}

bool DropDownWin32::create(IWidget* parent, const std::wstring& text, int style)
{
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("COMBOBOX"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST,
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

	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< DropDownWin32 >(this, &DropDownWin32::eventCommand));

	return true;
}

int DropDownWin32::add(const std::wstring& item)
{
	tstring tmp = wstots(item);
	return int(m_hWnd.sendMessage(CB_ADDSTRING, 0, (LPARAM)tmp.c_str()));
}

bool DropDownWin32::remove(int index)
{
	m_hWnd.sendMessage(CB_DELETESTRING, (WPARAM)index, 0);
	return true;
}

void DropDownWin32::removeAll()
{
	m_hWnd.sendMessage(CB_RESETCONTENT, 0, 0);
}

int DropDownWin32::count() const
{
	return int(m_hWnd.sendMessage(CB_GETCOUNT, 0, 0));
}

void DropDownWin32::set(int index, const std::wstring& item)
{
	remove(index);
	m_hWnd.sendMessage(CB_INSERTSTRING, (WPARAM)index, (LPARAM)wstots(item).c_str());
}

std::wstring DropDownWin32::get(int index) const
{
	TCHAR tmp[256];
	m_hWnd.sendMessage(CB_GETLBTEXT, (WPARAM)index, (LPARAM)tmp);
	return tstows(tmp);
}

void DropDownWin32::select(int index)
{
	m_hWnd.sendMessage(CB_SETCURSEL, (WPARAM)index, 0);
}

int DropDownWin32::getSelected() const
{
	LRESULT index = m_hWnd.sendMessage(CB_GETCURSEL, 0, 0);
	return (index != CB_ERR) ? int(index) : -1;
}

void DropDownWin32::setRect(const Rect& rect)
{
	Rect expand(rect);
	expand.bottom = expand.top + 8 * 16;
	WidgetWin32Impl::setRect(expand);
}

Size DropDownWin32::getPreferedSize() const
{
	int32_t dpi = m_hWnd.getSystemDPI();
	Size sz(
		(128 * dpi) / 96,
		(24 * dpi) / 96
	);

	HDC hDC = GetWindowDC(m_hWnd);
	if (hDC != NULL)
	{
		int32_t c = count();
		for (int32_t i = 0; i < c; ++i)
		{
			std::wstring s = get(i);
			if (!s.empty())
			{
				SIZE size = { 0 };
				GetTextExtentPoint32(hDC, wstots(s).c_str(), int(s.length()), &size);
				sz.cx = std::max< int32_t >(size.cx + (32 * dpi) / 96, sz.cx);
			}
		}
		ReleaseDC(m_hWnd, hDC);
	}

	return sz;
}

LRESULT DropDownWin32::eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
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

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/ListBoxWx.h"
#include "Ui/ListBox.h"

namespace traktor
{
	namespace ui
	{

ListBoxWx::ListBoxWx(EventSubject* owner)
:	WidgetWxImpl< IListBox, wxListBox >(owner)
{
}

bool ListBoxWx::create(IWidget* parent, int style)
{
	if (!parent)
		return false;
		
	int wxStyle = 0;
	if (style & WsBorder)
		wxStyle |= wxSIMPLE_BORDER;
	if (style & WsClientBorder)
		wxStyle |= wxSUNKEN_BORDER;
	if (!(style & (WsBorder | WsClientBorder)))
		wxStyle |= wxNO_BORDER;
	if (style & ListBox::WsMultiple)
		wxStyle |= wxLB_MULTIPLE;
	if (style & ListBox::WsExtended)
		wxStyle |= wxLB_EXTENDED;

	m_window = new wxListBox();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wxDefaultPosition,
		wxDefaultSize,
		wxArrayString(),
		wxStyle
	))
	{
		m_window->Destroy();
		return false;
	}

	if (!WidgetWxImpl< IListBox, wxListBox >::create(style))
		return false;

	T_CONNECT(m_window, wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEvent, ListBoxWx, &ListBoxWx::onSelected);

	return true;
}

int ListBoxWx::add(const std::wstring& item)
{
	return m_window->Append(wstots(item).c_str());
}

bool ListBoxWx::remove(int index)
{
	m_window->Delete(index);
	return true;
}

void ListBoxWx::removeAll()
{
	m_window->Clear();
}

int ListBoxWx::count() const
{
	return m_window->GetCount();
}

void ListBoxWx::set(int index, const std::wstring& item)
{
	m_window->SetString(index, wstots(item).c_str());
}

std::wstring ListBoxWx::get(int index) const
{
	return tstows((const wxChar*)m_window->GetString(index));
}

void ListBoxWx::select(int index)
{
	if (index >= 0 && index < int(m_window->GetCount()))
		m_window->Select(index);
}

bool ListBoxWx::selected(int index) const
{
	if (index < 0 || index >= count())
		return false;

	return m_window->IsSelected(index);
}

Rect ListBoxWx::getItemRect(int index) const
{
#if defined(_WIN32)
	RECT rc;
	SendMessage(
		(HWND)m_window->GetHWND(),
		LB_GETITEMRECT,
		(WPARAM)index,
		(LPARAM)&rc
	);
	return Rect(rc.left, rc.top, rc.right, rc.bottom);
#else
	return Rect(0, 0, 0, 0);
#endif
}

void ListBoxWx::onSelected(wxCommandEvent& event)
{
	SelectionChangeEvent selectionChangeEvent(m_owner);
	m_owner->raiseEvent(&selectionChangeEvent);
}

	}
}

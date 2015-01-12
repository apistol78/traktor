#include "Ui/Wx/DropDownWx.h"

namespace traktor
{
	namespace ui
	{

DropDownWx::DropDownWx(EventSubject* owner)
:	WidgetWxImpl< IDropDown, wxComboBox >(owner)
{
}

bool DropDownWx::create(IWidget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;
		
	int wxStyle = wxCB_READONLY;
	if (style & WsClientBorder)
		wxStyle |= wxSUNKEN_BORDER;
	else if (!(style & WsBorder))
		wxStyle |= wxNO_BORDER;

	m_window = new wxComboBox();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wstots(text).c_str(),
		wxDefaultPosition,
		wxDefaultSize,
		wxArrayString(),
		wxStyle
	))
	{
		m_window->Destroy();
		return false;
	}

	if (!WidgetWxImpl< IDropDown, wxComboBox >::create(style))
		return false;

	T_CONNECT(m_window, wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEvent, DropDownWx, &DropDownWx::onSelected);

	return true;
}

int DropDownWx::add(const std::wstring& item)
{
	return m_window->Append(wstots(item).c_str());
}

bool DropDownWx::remove(int index)
{
	m_window->Delete(index);
	return true;
}

void DropDownWx::removeAll()
{
	m_window->Clear();
}

int DropDownWx::count() const
{
	return m_window->GetCount();
}

void DropDownWx::set(int index, const std::wstring& item)
{
	m_window->SetString(index, wstots(item));
}

std::wstring DropDownWx::get(int index) const
{
	return tstows((const wxChar*)m_window->GetString(index));
}

void DropDownWx::select(int index)
{
	m_window->Select(index);
}

int DropDownWx::getSelected() const
{
	return m_window->GetSelection();
}

void DropDownWx::onSelected(wxCommandEvent& event)
{
	SelectionChangeEvent selectionChangeEvent(m_owner);
	m_owner->raiseEvent(&selectionChangeEvent);
}

	}
}

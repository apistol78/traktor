#include "Ui/Wx/ComboBoxWx.h"

namespace traktor
{
	namespace ui
	{

ComboBoxWx::ComboBoxWx(EventSubject* owner)
:	WidgetWxImpl< IComboBox, wxComboBox >(owner)
{
}

bool ComboBoxWx::create(IWidget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;
		
	int wxStyle = 0;
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

	if (!WidgetWxImpl< IComboBox, wxComboBox >::create(style))
		return false;

	T_CONNECT(m_window, wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEvent, ComboBoxWx, &ComboBoxWx::onSelected);

	return true;
}

int ComboBoxWx::add(const std::wstring& item)
{
	return m_window->Append(wstots(item).c_str());
}

bool ComboBoxWx::remove(int index)
{
	m_window->Delete(index);
	return true;
}

void ComboBoxWx::removeAll()
{
	m_window->Clear();
}

int ComboBoxWx::count() const
{
	return m_window->GetCount();
}

std::wstring ComboBoxWx::get(int index) const
{
	return tstows((const wxChar*)m_window->GetString(index));
}

void ComboBoxWx::select(int index)
{
	m_window->Select(index);
}

int ComboBoxWx::getSelected() const
{
	return m_window->GetSelection();
}

void ComboBoxWx::onSelected(wxCommandEvent& event)
{
	SelectionChangeEvent selectionChangeEvent(m_owner);
	m_owner->raiseEvent(&selectionChangeEvent);
}

	}
}

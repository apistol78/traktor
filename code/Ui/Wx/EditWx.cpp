#include "Ui/Wx/EditWx.h"

namespace traktor
{
	namespace ui
	{

EditWx::EditWx(EventSubject* owner)
:	WidgetWxImpl< IEdit, wxTextCtrl >(owner)
{
}

bool EditWx::create(IWidget* parent, const std::wstring& text, int style)
{
	int wxStyle = 0;
	if (style & WsClientBorder)
		wxStyle |= wxSUNKEN_BORDER;
	else if (!(style & WsBorder))
		wxStyle |= wxNO_BORDER;

	m_window = new wxTextCtrl();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wxT(""),
		wxDefaultPosition,
		wxDefaultSize,
		wxStyle
	))
	{
		m_window->Destroy();
		return false;
	}

	if (!WidgetWxImpl< IEdit, wxTextCtrl >::create(style))
		return false;

	setText(text);
	return true;
}

void EditWx::setSelection(int from, int to)
{
	m_window->SetSelection(from, to);
}

void EditWx::getSelection(int& outFrom, int& outTo) const
{
	long from, to;
	m_window->GetSelection(&from, &to);
	outFrom = int(from);
	outTo = int(to);
}

void EditWx::selectAll()
{
	m_window->SetSelection(-1, -1);
}

void EditWx::setText(const std::wstring& text)
{
	tstring ttext = wstots(text);
	m_window->SetValue(ttext.c_str());
}

std::wstring EditWx::getText() const
{
	wxString value = m_window->GetValue();
	return tstows((const wxChar*)value);
}

	}
}

#include "Ui/Wx/ContainerWx.h"

namespace traktor
{
	namespace ui
	{

ContainerWx::ContainerWx(EventSubject* owner)
:	WidgetWxImpl< IContainer, wxWindow >(owner)
{
}

bool ContainerWx::create(IWidget* parent, int style)
{
	int wxStyle = wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE;

	if (style & WsBorder)
		wxStyle |= wxSIMPLE_BORDER;
	if (style & WsClientBorder)
		wxStyle |= wxSUNKEN_BORDER;

	m_window = new wxWindow(
		parent ? static_cast< wxWindow* >(parent->getInternalHandle()) : 0,
		-1,
		wxDefaultPosition,
		wxDefaultSize,
		wxStyle
	);

	if (!WidgetWxImpl< IContainer, wxWindow >::create(style))
		return false;

	return true;
}

void ContainerWx::setText(const std::wstring& text)
{
	m_text = text;
}

std::wstring ContainerWx::getText() const
{
	return m_text;
}

	}
}

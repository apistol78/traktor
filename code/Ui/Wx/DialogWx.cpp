/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/DialogWx.h"

namespace traktor
{
	namespace ui
	{

DialogWx::DialogWx(EventSubject* owner)
:	WidgetWxImpl< IDialog, wxDialog >(owner)
{
}

bool DialogWx::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	m_window = new wxDialog();
	
	if (!m_window->Create(
		parent ? static_cast< wxWindow* >(parent->getInternalHandle()) : 0,
		-1,
		wstots(text).c_str(),
		wxDefaultPosition,
		wxSize(width, height),
		wxDEFAULT_DIALOG_STYLE | ((style & WsResizable) ? wxRESIZE_BORDER : 0) | (parent ? 0 : wxDIALOG_NO_PARENT)
	))
	{
		m_window->Destroy();
		return false;
	}
	
	if (!WidgetWxImpl< IDialog, wxDialog >::create(0))
		return false;

	return true;
}

void DialogWx::setIcon(drawing::Image* icon)
{
}

int DialogWx::showModal()
{
	m_window->Centre();
	return m_window->ShowModal();
}

void DialogWx::endModal(int result)
{
	m_window->EndModal(result);
}

void DialogWx::setMinSize(const Size& minSize)
{
	m_window->SetMinSize(wxSize(minSize.cx, minSize.cy));
}

	}
}

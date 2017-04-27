/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/tglbtn.h>
#include "Ui/Wx/ButtonWx.h"
#include "Ui/Button.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

ButtonWx::ButtonWx(EventSubject* owner)
:	WidgetWxImpl< IButton, wxControl >(owner)
{
}

bool ButtonWx::create(IWidget* parent, const std::wstring& text, int style)
{
	tstring tmp = wstots(text);

	if (!(style & Button::WsToggle))
	{
		m_window = new wxButton();
		
		if (!static_cast< wxButton* >(m_window)->Create(
			static_cast< wxWindow* >(parent->getInternalHandle()),
			0,
			tmp.c_str()
		))
		{
			m_window->Destroy();
			return false;
		}
		
		if (style & Button::WsDefaultButton)
			static_cast< wxButton* >(m_window)->SetDefault();

		T_CONNECT(m_window, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEvent, ButtonWx, &ButtonWx::onClick);
	}
	else
	{
		m_window = new wxToggleButton();

		if (!static_cast< wxToggleButton* >(m_window)->Create(
			static_cast< wxWindow* >(parent->getInternalHandle()),
			0,
			tmp.c_str()
		))
		{
			m_window->Destroy();
			return false;
		}

		T_CONNECT(m_window, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEvent, ButtonWx, &ButtonWx::onClick);
	}

	if (!WidgetWxImpl< IButton, wxControl >::create(0))
		return false;

	return true;
}

void ButtonWx::setState(bool state)
{
	static_cast< wxToggleButton* >(m_window)->SetValue(state);
}

bool ButtonWx::getState() const
{
	return static_cast< wxToggleButton* >(m_window)->GetValue();
}

void ButtonWx::onClick(wxCommandEvent& event)
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

	}
}

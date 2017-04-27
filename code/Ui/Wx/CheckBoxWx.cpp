/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/CheckBoxWx.h"

namespace traktor
{
	namespace ui
	{

CheckBoxWx::CheckBoxWx(EventSubject* owner) :
	WidgetWxImpl< ICheckBox, wxCheckBox >(owner)
{
}

bool CheckBoxWx::create(IWidget* parent, const std::wstring& text, bool checked)
{
	m_window = new wxCheckBox();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wstots(text).c_str()
	))
	{
		m_window->Destroy();
		return false;
	}
	
	if (!WidgetWxImpl< ICheckBox, wxCheckBox >::create(0))
		return false;

	T_CONNECT(m_window, wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEvent, CheckBoxWx, &CheckBoxWx::onClicked);

	return true;
}

void CheckBoxWx::setChecked(bool checked)
{
	static_cast< wxCheckBox* >(m_window)->SetValue(checked);
}

bool CheckBoxWx::isChecked() const
{
	return static_cast< wxCheckBox* >(m_window)->IsChecked();
}

void CheckBoxWx::onClicked(wxCommandEvent& event)
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

	}
}

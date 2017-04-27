/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/StaticWx.h"

namespace traktor
{
	namespace ui
	{

StaticWx::StaticWx(EventSubject* owner)
:	WidgetWxImpl< IStatic, wxStaticText >(owner)
{
}

bool StaticWx::create(IWidget* parent, const std::wstring& text)
{
	m_window = new wxStaticText();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wstots(text).c_str()
	))
	{
		m_window->Destroy();
		return false;
	}

	if (!WidgetWxImpl< IStatic, wxStaticText >::create(0))
		return false;
	
	return true;
}

Size StaticWx::getPreferedSize() const
{
	return getTextExtent(getText());
}

	}
}

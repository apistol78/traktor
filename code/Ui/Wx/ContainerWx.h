/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ContainerWx_H
#define traktor_ui_ContainerWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerWx : public WidgetWxImpl< IContainer, wxWindow >
{
public:
	ContainerWx(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setText(const std::wstring& text);

	virtual std::wstring getText() const;

private:
	std::wstring m_text;
};

	}
}

#endif	// traktor_ui_ContainerWx_H

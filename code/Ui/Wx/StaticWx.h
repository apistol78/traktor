/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_StaticWx_H
#define traktor_ui_StaticWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IStatic.h"

namespace traktor
{
	namespace ui
	{

class StaticWx : public WidgetWxImpl< IStatic, wxStaticText >
{
public:
	StaticWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text);

	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_StaticWx_H

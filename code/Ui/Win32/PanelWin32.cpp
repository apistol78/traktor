/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Win32/PanelWin32.h"

namespace traktor
{
	namespace ui
	{

PanelWin32::PanelWin32(EventSubject* owner) :
	WidgetWin32Impl< IPanel >(owner)
{
}

bool PanelWin32::create(IWidget* parent, const std::wstring& text)
{
	return false;
}

void PanelWin32::update(const Rect* rc, bool immediate)
{
}

Rect PanelWin32::getInnerRect() const
{
	Rect rc = WidgetWin32Impl< IPanel >::getInnerRect();
	rc.left += 8;
	rc.top += 16;
	rc.right -= 8;
	rc.bottom -= 8;
	return rc;
}

Size PanelWin32::getMinimumSize() const
{
	return WidgetWin32Impl< IPanel >::getMinimumSize() + Size(16, 24);
}

Size PanelWin32::getPreferedSize() const
{
	return WidgetWin32Impl< IPanel >::getPreferedSize() + Size(16, 24);
}

Size PanelWin32::getMaximumSize() const
{
	return WidgetWin32Impl< IPanel >::getMaximumSize() + Size(16, 24);
}

	}
}

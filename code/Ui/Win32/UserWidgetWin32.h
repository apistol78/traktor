/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_UserWidgetWin32_H
#define traktor_ui_UserWidgetWin32_H

#include "Ui/Itf/IUserWidget.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class UserWidgetWin32 : public WidgetWin32Impl< IUserWidget >
{
public:
	UserWidgetWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

private:
	LRESULT eventButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_UserWidgetWin32_H

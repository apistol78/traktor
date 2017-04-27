/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ButtonWin32_H
#define traktor_ui_ButtonWin32_H

#include "Ui/Itf/IButton.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class ButtonWin32 : public WidgetWin32Impl< IButton >
{
public:
	ButtonWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setState(bool state);

	virtual bool getState() const;

	virtual Size getPreferedSize() const;

private:
	LRESULT eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_ButtonWin32_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_SliderWin32_H
#define traktor_ui_SliderWin32_H

#include "Ui/Itf/ISlider.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class SliderWin32 : public WidgetWin32Impl< ISlider >
{
public:
	SliderWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual void setRange(int minValue, int maxValue);

	virtual void setValue(int value);

	virtual int getValue() const;

	virtual Size getPreferedSize() const;

private:
	LRESULT eventScroll(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_SliderWin32_H

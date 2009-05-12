#ifndef traktor_ui_SliderWin32_H
#define traktor_ui_SliderWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/ISlider.h"

namespace traktor
{
	namespace ui
	{

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

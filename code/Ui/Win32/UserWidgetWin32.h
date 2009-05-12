#ifndef traktor_ui_UserWidgetWin32_H
#define traktor_ui_UserWidgetWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

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

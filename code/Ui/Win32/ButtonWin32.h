#ifndef traktor_ui_ButtonWin32_H
#define traktor_ui_ButtonWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IButton.h"

namespace traktor
{
	namespace ui
	{

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

#ifndef traktor_ui_CheckBoxWin32_H
#define traktor_ui_CheckBoxWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/ICheckBox.h"

namespace traktor
{
	namespace ui
	{

class CheckBoxWin32 : public WidgetWin32Impl< ICheckBox >
{
public:
	CheckBoxWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;

	virtual Size getPreferedSize() const;

private:
	LRESULT eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_CheckBoxWin32_H

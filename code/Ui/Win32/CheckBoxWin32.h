#ifndef traktor_ui_CheckBoxWin32_H
#define traktor_ui_CheckBoxWin32_H

#include "Ui/Itf/ICheckBox.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class CheckBoxWin32 : public WidgetWin32Impl< ICheckBox >
{
public:
	CheckBoxWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;

	virtual Size getPreferedSize() const;

private:
	SmartBrush m_brushBackground;

	LRESULT eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventCtlColorStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventCtlColorBtn(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_CheckBoxWin32_H

#ifndef traktor_ui_EditWin32_H
#define traktor_ui_EditWin32_H

#include "Ui/Itf/IEdit.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class EditWin32 : public WidgetWin32Impl< IEdit >
{
public:
	EditWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setSelection(int from, int to);

	virtual void getSelection(int& outFrom, int& outTo) const;

	virtual void selectAll();

	virtual Size getPreferedSize() const;

private:
	SmartBrush m_brushBackground;
	SmartBrush m_brushBackgroundDisabled;

	LRESULT eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventCtlColorStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventCtlColorEdit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_EditWin32_H

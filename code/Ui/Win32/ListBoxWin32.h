#ifndef traktor_ui_ListBoxWin32_H
#define traktor_ui_ListBoxWin32_H

#include "Ui/Itf/IListBox.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class ListBoxWin32 : public WidgetWin32Impl< IListBox >
{
public:
	ListBoxWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

	virtual int add(const std::wstring& item);

	virtual bool remove(int index);

	virtual void removeAll();

	virtual int count() const;

	virtual void set(int index, const std::wstring& item);

	virtual std::wstring get(int index) const;
	
	virtual void select(int index);

	virtual bool selected(int index) const;

	virtual Rect getItemRect(int index) const;

private:
	SmartBrush m_brushBackground;
	bool m_single;

	LRESULT eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventCtlColorListBox(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_ListBoxWin32_H

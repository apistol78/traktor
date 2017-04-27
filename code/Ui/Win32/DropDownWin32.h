/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_DropDownWin32_H
#define traktor_ui_DropDownWin32_H

#include "Ui/Itf/IDropDown.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class DropDownWin32 : public WidgetWin32Impl< IDropDown >
{
public:
	DropDownWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual int add(const std::wstring& item);

	virtual bool remove(int index);

	virtual void removeAll();

	virtual int count() const;

	virtual void set(int index, const std::wstring& item);

	virtual std::wstring get(int index) const;
	
	virtual void select(int index);

	virtual int getSelected() const;

	virtual void setRect(const Rect& rect);

	virtual Size getPreferedSize() const;

private:
	LRESULT eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_DropDownWin32_H

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MenuBarWin32_H
#define traktor_ui_MenuBarWin32_H

#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Core/RefArray.h"
#include "Ui/Itf/IMenuBar.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief
 * \ingroup UIW32
 */
class MenuBarWin32 : public IMenuBar
{
public:
	MenuBarWin32(EventSubject* owner);

	virtual bool create(IForm* form);

	virtual void destroy();

	virtual void add(MenuItem* item);

private:
	friend class FormWin32;

	EventSubject* m_owner;
	FormWin32* m_form;
	HMENU m_hMenu;
	RefArray< MenuItem > m_menuItems;

	LRESULT eventInitMenuPopup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_MenuBarWin32_H

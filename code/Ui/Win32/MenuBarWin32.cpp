#include "Core/Misc/TString.h"
#include "Ui/Bitmap.h"
#include "Ui/MenuItem.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IForm.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/FormWin32.h"
#include "Ui/Win32/MenuBarWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

#if !defined(WINCE)

HMENU buildMenu(MenuItem* item)
{
	if (item->count() <= 0)
		return 0;

	MENUITEMINFO mii;
	MENUINFO mi;
	tstring ts;

	HMENU hMenu = CreateMenu();
	if (!hMenu)
		return 0;

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_MENUDATA;
	mi.dwMenuData = (ULONG_PTR)item;
	SetMenuInfo(hMenu, &mi);

	for (int i = 0; i < item->count(); ++i)
	{
		Ref< MenuItem > child = item->get(i);
		T_ASSERT (child);

		memset(&mii, 0, sizeof(mii));
		mii.cbSize = sizeof(mii);

		if (child->getText() != L"-")
		{
			ts = wstots(child->getText());
			mii.fMask = MIIM_STRING | MIIM_DATA | (child->count() > 0 ? MIIM_SUBMENU : 0);
			mii.dwTypeData = (LPTSTR)ts.c_str();
			mii.cch = (UINT)ts.length();
			mii.hSubMenu = buildMenu(child);
			mii.dwItemData = (ULONG_PTR)child.ptr();
		}
		else
		{
			mii.fMask = MIIM_FTYPE;
			mii.fType = MFT_SEPARATOR;
		}

		InsertMenuItem(hMenu, i, TRUE, &mii);
	}

	return hMenu;
}

#endif

		}

MenuBarWin32::MenuBarWin32(EventSubject* owner)
:	m_owner(owner)
,	m_form(0)
,	m_hMenu(NULL)
{
}

bool MenuBarWin32::create(IForm* form)
{
	m_hMenu = CreateMenu();
	if (!m_hMenu)
		return false;

#if !defined(WINCE)

	MENUINFO mi;

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(m_hMenu, &mi);

	m_form = static_cast< FormWin32* >(form);
	m_form->registerMenuBar(this);

	HWND hWndParent = static_cast< HWND >(m_form->getInternalHandle());
	T_ASSERT_M (hWndParent, L"Invalid parent handle");

	SetMenu(hWndParent, m_hMenu);
	DrawMenuBar(hWndParent);

#endif

	return true;
}

void MenuBarWin32::destroy()
{
	if (!m_form)
		return;

	HWND hWndParent = static_cast< HWND >(m_form->getInternalHandle());

#if !defined(WINCE)

	m_form->unregisterMenuBar(this);
	m_form = 0;

	SetMenu(hWndParent, NULL);

#endif

	DrawMenuBar(hWndParent);
	DestroyMenu(m_hMenu);

	m_hMenu = NULL;

	delete this;
}

void MenuBarWin32::add(MenuItem* item)
{
#if !defined(WINCE)

	MENUINFO mi;
	MENUITEMINFO mii;
	HMENU hSubMenu = NULL;

	// Create dummy sub-menu which holds a pointer to our item.
	if (item->count() > 0)
	{
		hSubMenu = CreateMenu();
		T_ASSERT (hSubMenu);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_MENUDATA;
		mi.dwMenuData = (ULONG_PTR)item;
		SetMenuInfo(hSubMenu, &mi);
	}

	// Create "top" popup menu.
	tstring tmp = wstots(item->getText());

	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STRING | (hSubMenu != NULL ? MIIM_SUBMENU : 0);
	mii.dwTypeData = (LPTSTR)tmp.c_str();
	mii.cch = (UINT)tmp.length();
	mii.hSubMenu = hSubMenu;

	if (item->getImage())
	{
		mii.fMask |= MIIM_BITMAP;
		mii.hbmpItem = static_cast< BitmapWin32* >(item->getImage()->getIBitmap())->getHBitmap();
	}

	HWND hWndParent = static_cast< HWND >(m_form->getInternalHandle());

	InsertMenuItem(m_hMenu, UINT(m_menuItems.size()), TRUE, &mii);
	DrawMenuBar(hWndParent);

	m_menuItems.push_back(item);

#endif
}

LRESULT MenuBarWin32::eventInitMenuPopup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
#if !defined(WINCE)

	MENUINFO mi;
	MENUITEMINFO mii;
	tstring ts;

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_MENUDATA;
	GetMenuInfo((HMENU)wParam, &mi);

	MenuItem* item = reinterpret_cast< MenuItem* >(mi.dwMenuData);
	if (!item)
		return 0;

	// Remove all items in this menu.
	while (GetMenuItemCount((HMENU)wParam) > 0)
		RemoveMenu((HMENU)wParam, 0, MF_BYPOSITION);

	// Rebuild menu from child items.
	for (int i = 0; i < item->count(); ++i)
	{
		Ref< MenuItem > child = item->get(i);
		T_ASSERT (child);

		memset(&mii, 0, sizeof(mii));
		mii.cbSize = sizeof(mii);

		if (child->getText() != L"-")
		{
			ts = wstots(child->getText());
			mii.fMask = MIIM_STRING | MIIM_DATA | (child->count() > 0 ? MIIM_SUBMENU : 0);
			mii.dwTypeData = (LPTSTR)ts.c_str();
			mii.cch = (UINT)ts.length();
			mii.hSubMenu = buildMenu(child);
			mii.dwItemData = (ULONG_PTR)child.ptr();
		}
		else
		{
			mii.fMask = MIIM_FTYPE;
			mii.fType = MFT_SEPARATOR;
		}

		InsertMenuItem((HMENU)wParam, i, TRUE, &mii);
	}

	pass = false;

#endif
	return 0;
}

LRESULT MenuBarWin32::eventMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
#if !defined(WINCE)

	MENUITEMINFO mii;

	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_DATA;

	if (!GetMenuItemInfo((HMENU)lParam, (UINT)wParam, TRUE, &mii))
		return 0;

	MenuItem* item = reinterpret_cast< MenuItem* >(mii.dwItemData);
	T_ASSERT (item);

	MenuClickEvent menuClickEvent(m_owner, item, item->getCommand());
	m_owner->raiseEvent(&menuClickEvent);

	pass = !menuClickEvent.consumed();

#endif
	return 0;
}

	}
}

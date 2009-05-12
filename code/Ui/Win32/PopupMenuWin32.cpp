#include "Ui/Win32/PopupMenuWin32.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/MenuItem.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

bool PopupMenuWin32::create()
{
	return true;
}

void PopupMenuWin32::destroy()
{
	delete this;
}

void PopupMenuWin32::add(MenuItem* item)
{
	m_items.push_back(item);
}

MenuItem* PopupMenuWin32::show(IWidget* parent, const Point& at)
{
	if (!parent)
		return 0;

	HWND hWnd = (HWND)parent->getInternalHandle();
	if (!hWnd)
		return 0;

	HMENU hMenu = CreatePopupMenu();
	if (!hMenu)
		return 0;

	for (RefArray< MenuItem >::iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		tstring tmp = wstots((*i)->getText());	
		UINT flags = (*i)->getText() != L"-" ? MF_STRING : MF_SEPARATOR;

#if !defined(WINCE)
		if (!(*i)->isEnable())
			flags |= MF_DISABLED;
#endif

		if (!(*i)->count())
		{
			AppendMenu(
				hMenu,
				flags,
				UINT(*i),
				(LPTSTR)tmp.c_str()
			);
		}
		else
		{
			HMENU hSubMenu = buildMenu((*i));
			AppendMenu(
				hMenu,
				flags | MF_POPUP,
				UINT(hSubMenu),
				(LPTSTR)tmp.c_str()
			);
		}
	}

	POINT pnt = { at.x, at.y };
	ClientToScreen(hWnd, &pnt);

	UINT id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, pnt.x, pnt.y, hWnd, NULL);

	DestroyMenu(hMenu);

	return id ? reinterpret_cast< MenuItem* >(id) : 0;
}

HMENU PopupMenuWin32::buildMenu(MenuItem* item)
{
	T_ASSERT (item->count() > 0);

	HMENU hMenu = CreateMenu();

	for (int i = 0; i < item->count(); ++i)
	{
		Ref< MenuItem > subItem = item->get(i);
		tstring tmp = wstots(subItem->getText());

		UINT flags = subItem->getText() != L"-" ? MF_STRING : MF_SEPARATOR;

#if !defined(WINCE)
		if (!subItem->isEnable())
			flags |= MF_DISABLED;
#endif

		if (!subItem->count())
		{
			AppendMenu(
				hMenu,
				flags,
				UINT(subItem.getPtr()),
				(LPTSTR)tmp.c_str()
			);
		}
		else
		{
			HMENU hSubMenu = buildMenu(subItem);
			AppendMenu(
				hMenu,
				flags | MF_POPUP,
				UINT(hSubMenu),
				(LPTSTR)tmp.c_str()
			);
		}
	}

	return hMenu;
}

	}
}

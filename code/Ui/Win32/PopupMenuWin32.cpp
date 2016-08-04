#include "Core/Misc/TString.h"
#include "Ui/MenuItem.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Win32/PopupMenuWin32.h"

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
	HWND hWnd = parent ? (HWND)parent->getInternalHandle() : NULL;

	HMENU hMenu = CreatePopupMenu();
	if (!hMenu)
		return 0;

	for (uint32_t i = 0; i < m_items.size(); ++i)
	{
		MenuItem* item = m_items[i];
		T_ASSERT (item);

		tstring tmp = wstots(item->getText());	
		UINT flags = item->getText() != L"-" ? MF_STRING : MF_SEPARATOR;

		if (!item->isEnable())
			flags |= MF_DISABLED;

		if (item->getCheckBox())
		{
			if (item->isChecked())
				flags |= MF_CHECKED;
			else
				flags |= MF_UNCHECKED;
		}

		if (!item->count())
		{
			AppendMenu(
				hMenu,
				flags,
				UINT_PTR(m_flatten.size() + 1),
				(LPTSTR)tmp.c_str()
			);
			m_flatten.push_back(item);
		}
		else
		{
			HMENU hSubMenu = buildMenu(item);
			AppendMenu(
				hMenu,
				flags | MF_POPUP,
				UINT_PTR(hSubMenu),
				(LPTSTR)tmp.c_str()
			);
		}
	}

	POINT pnt = { at.x, at.y };
	if (hWnd != NULL)
		ClientToScreen(hWnd, &pnt);

	Window containerWindow;
	containerWindow.create(
		hWnd,
		_T("TraktorWin32Class"),
		_T(""),
		WS_POPUP | (hWnd ? WS_CHILD : 0),
		0,
		0, 0, 0, 0
	);

	int32_t id = (int32_t)TrackPopupMenuEx(
		hMenu,
		TPM_RETURNCMD,
		pnt.x,
		pnt.y,
		containerWindow,
		NULL
	);

	if (id > 0 && id <= int32_t(m_flatten.size()))
	{
		MenuItem* item = m_flatten[id - 1];

		if (item && item->getCheckBox())
			item->setChecked(!item->isChecked());

		DestroyMenu(hMenu);

		m_flatten.clear();
		return item;
	}
	else
	{
		m_flatten.clear();
		return 0;
	}
}

HMENU PopupMenuWin32::buildMenu(MenuItem* item)
{
	T_ASSERT (item->count() > 0);

	HMENU hMenu = CreateMenu();

	for (int i = 0; i < item->count(); ++i)
	{
		MenuItem* subItem = item->get(i);

		tstring tmp = wstots(subItem->getText());
		UINT flags = subItem->getText() != L"-" ? MF_STRING : MF_SEPARATOR;

		if (!subItem->isEnable())
			flags |= MF_DISABLED;

		if (subItem->getCheckBox())
		{
			if (subItem->isChecked())
				flags |= MF_CHECKED;
			else
				flags |= MF_UNCHECKED;
		}

		if (!subItem->count())
		{
			AppendMenu(
				hMenu,
				flags,
				UINT_PTR(m_flatten.size() + 1),
				(LPTSTR)tmp.c_str()
			);
			m_flatten.push_back(subItem);
		}
		else
		{
			HMENU hSubMenu = buildMenu(subItem);
			AppendMenu(
				hMenu,
				flags | MF_POPUP,
				UINT_PTR(hSubMenu),
				(LPTSTR)tmp.c_str()
			);
		}
	}

	return hMenu;
}

	}
}

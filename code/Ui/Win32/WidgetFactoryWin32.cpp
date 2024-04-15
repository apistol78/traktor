/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <set>
#include "Ui/Win32/WidgetFactoryWin32.h"
#include "Ui/Win32/DialogWin32.h"
#include "Ui/Win32/EventLoopWin32.h"
#include "Ui/Win32/FormWin32.h"
#include "Ui/Win32/NotificationIconWin32.h"
#include "Ui/Win32/PathDialogWin32.h"
#include "Ui/Win32/ToolFormWin32.h"
#include "Ui/Win32/UserWidgetWin32.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/ClipboardWin32.h"

namespace traktor::ui
{
	namespace
	{

int CALLBACK enumFontCallBack(const LOGFONT* lf, const TEXTMETRIC*, DWORD, LPVOID lParam)
{
	std::set< std::wstring >& outFonts = *reinterpret_cast< std::set< std::wstring >* >(lParam);
	outFonts.insert(lf->lfFaceName);
	return TRUE;
}

BOOL enumMonitors(HMONITOR hMonitor, HDC hDC, LPRECT lpRect, LPARAM lpUser)
{
	std::vector< HMONITOR >* monitors = (std::vector< HMONITOR >*)lpUser;
	monitors->push_back(hMonitor);
	return TRUE;
}

	}

IEventLoop* WidgetFactoryWin32::createEventLoop(EventSubject* owner)
{
	return new EventLoopWin32();
}

IDialog* WidgetFactoryWin32::createDialog(EventSubject* owner)
{
	return new DialogWin32(owner);
}

IForm* WidgetFactoryWin32::createForm(EventSubject* owner)
{
	return new FormWin32(owner);
}

INotificationIcon* WidgetFactoryWin32::createNotificationIcon(EventSubject* owner)
{
	return new NotificationIconWin32(owner);
}

IPathDialog* WidgetFactoryWin32::createPathDialog(EventSubject* owner)
{
	return new PathDialogWin32(owner);
}

IToolForm* WidgetFactoryWin32::createToolForm(EventSubject* owner)
{
	return new ToolFormWin32(owner);
}

IUserWidget* WidgetFactoryWin32::createUserWidget(EventSubject* owner)
{
	return new UserWidgetWin32(owner);
}

ISystemBitmap* WidgetFactoryWin32::createBitmap()
{
	return new BitmapWin32();
}

IClipboard* WidgetFactoryWin32::createClipboard()
{
	return new ClipboardWin32();
}

void WidgetFactoryWin32::getSystemFonts(std::list< std::wstring >& outFonts)
{
	HDC hDC = GetDC(NULL);

	LOGFONT lf = { 0 };
	lf.lfCharSet = DEFAULT_CHARSET;

	std::set< std::wstring > fonts;
	EnumFontFamiliesEx(
		hDC,
		&lf,
		(FONTENUMPROC)enumFontCallBack,
		(LPARAM)&fonts,
		0
	);

	ReleaseDC(NULL, hDC);

	outFonts = std::list< std::wstring >(fonts.begin(), fonts.end());
}

void WidgetFactoryWin32::getDesktopRects(std::list< Rect >& outRects) const
{
	std::vector< HMONITOR > monitors;
	EnumDisplayMonitors(NULL, NULL, &enumMonitors, (LPARAM)&monitors);
	for (auto monitor : monitors)
	{
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitor, &mi);
		outRects.push_back(Rect(
			mi.rcWork.left,
			mi.rcWork.top,
			mi.rcWork.right,
			mi.rcWork.bottom
		));
	}
}

}

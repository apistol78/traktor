/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Win32/WidgetFactoryWin32.h"
#include "Ui/Win32/ContainerWin32.h"
#include "Ui/Win32/DialogWin32.h"
#include "Ui/Win32/EventLoopWin32.h"
#include "Ui/Win32/FormWin32.h"
#include "Ui/Win32/NotificationIconWin32.h"
#include "Ui/Win32/PathDialogWin32.h"
#include "Ui/Win32/ToolFormWin32.h"
#include "Ui/Win32/UserWidgetWin32.h"
#include "Ui/Win32/WebBrowserWin32.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/ClipboardWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

int CALLBACK enumFontCallBack(const LOGFONT* lf, const TEXTMETRIC*, DWORD, LPVOID lParam)
{
	std::set< std::wstring >& outFonts = *reinterpret_cast< std::set< std::wstring >* >(lParam);
	outFonts.insert(lf->lfFaceName);
	return TRUE; 
}

		}

WidgetFactoryWin32::WidgetFactoryWin32()
:	m_systemDPI(96)
{
	HDC hDC = GetDC(NULL);
	if (hDC != NULL)
	{
		m_systemDPI = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(NULL, hDC);
	}
}

IEventLoop* WidgetFactoryWin32::createEventLoop(EventSubject* owner)
{
	return new EventLoopWin32();
}

IContainer* WidgetFactoryWin32::createContainer(EventSubject* owner)
{
	return new ContainerWin32(owner);
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

IWebBrowser* WidgetFactoryWin32::createWebBrowser(EventSubject* owner)
{
	return new WebBrowserWin32(owner);
}

ISystemBitmap* WidgetFactoryWin32::createBitmap()
{
	return new BitmapWin32();
}

IClipboard* WidgetFactoryWin32::createClipboard()
{
	return new ClipboardWin32();
}

int32_t WidgetFactoryWin32::getSystemDPI() const
{
	return m_systemDPI;
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

	}
}

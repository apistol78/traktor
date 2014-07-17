#include "Core/Log/Log.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/FormWin32.h"
#include "Ui/Win32/MenuBarWin32.h"

namespace traktor
{
	namespace ui
	{

FormWin32::FormWin32(EventSubject* owner)
:	WidgetWin32Impl< IForm >(owner)
#if !defined(WINCE)
,	m_menuBar(0)
#endif
{
}

bool FormWin32::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
#if defined(WINCE)
	width =
	height = CW_USEDEFAULT;
#endif

#if !defined(WINCE)
	DWORD nativeStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (style & WsResizable)
		nativeStyle |= WS_THICKFRAME;
	else
		nativeStyle |= WS_DLGFRAME;

	if (style & WsSystemBox)
		nativeStyle |= WS_SYSMENU | WS_CAPTION;
	if (style & WsMinimizeBox)
		nativeStyle |= WS_MINIMIZEBOX;
	if (style & WsMaximizeBox)
		nativeStyle |= WS_MAXIMIZEBOX;
	if (style & WsCaption)
		nativeStyle |= WS_CAPTION;

#else
	DWORD nativeStyle = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
#endif

	if (parent)
		nativeStyle |= WS_CHILD;

	if (!m_hWnd.create(
		parent ? reinterpret_cast< HWND >(parent->getInternalHandle()) : NULL,
		_T("TraktorWin32Class"),
		wstots(text).c_str(),
		nativeStyle,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(
		WM_INITMENUPOPUP,
		new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventInitMenuPopup)
	);

#if !defined(WINCE)
	m_hWnd.registerMessageHandler(
		WM_MENUCOMMAND,
		new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventMenuCommand)
	);
#endif

	m_hWnd.registerMessageHandler(
		WM_CLOSE,
		new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventClose)
	);

	m_hWnd.registerMessageHandler(
		WM_DESTROY,
		new MethodMessageHandler< FormWin32 >(this, &FormWin32::eventDestroy)
	);

#if defined(WINCE)
	SHMENUBARINFO mbi;

	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = m_hWnd;
	mbi.nToolBarId = 0; //IDR_MENU;
	mbi.hInstRes = g_hInstance;

	if (SHCreateMenuBar(&mbi)) 
		m_hWndMenuBar = mbi.hwndMB;
	else
		log::error << L"Unable to create menu bar; SHCreateMenuBar failed" << Endl;
#endif

	return true;
}

void FormWin32::destroy()
{
	WidgetWin32Impl< IForm >::destroy();
}

void FormWin32::setIcon(IBitmap* icon)
{
	ICONINFO ii;

	BitmapWin32* bm = static_cast< BitmapWin32* >(icon);

	BitmapWin32 bmMask;
	bmMask.create(bm->getSize().cx, bm->getSize().cy);
	
	uint32_t* pMaskBits = static_cast< uint32_t* >(bmMask.getBits());
	uint32_t* pBits = static_cast< uint32_t* >(bm->getBits());

	for (uint32_t i = 0; i < uint32_t(bm->getSize().cx * bm->getSize().cy); ++i)
		pMaskBits[i] = pBits[i] != pBits[0] ? 0x00000000 : 0x00ffffff;

#if !defined(WINCE)
	GdiFlush();
#endif

	memset(&ii, 0, sizeof(ii));
	ii.fIcon = TRUE;
	ii.hbmMask = bmMask.getHBitmap();
	ii.hbmColor = bm->getHBitmap();

	HICON hIcon = CreateIconIndirect(&ii);
	T_ASSERT (hIcon);

	m_hWnd.sendMessage(WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	m_hWnd.sendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}

void FormWin32::maximize()
{
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);
}

void FormWin32::minimize()
{
	ShowWindow(m_hWnd, SW_MINIMIZE);
	UpdateWindow(m_hWnd);
}

void FormWin32::restore()
{
	ShowWindow(m_hWnd, SW_RESTORE);
	UpdateWindow(m_hWnd);
}

bool FormWin32::isMaximized() const
{
#if !defined(WINCE)
	BOOL zoomed = IsZoomed(m_hWnd);
#else
	BOOL zoomed = TRUE;
#endif
	return bool(zoomed == TRUE);
}

bool FormWin32::isMinimized() const
{
#if !defined(WINCE)
	BOOL iconic = IsIconic(m_hWnd);
#else
	BOOL iconic = FALSE;
#endif
	return bool(iconic == TRUE);
}

#if !defined(WINCE)

void FormWin32::registerMenuBar(MenuBarWin32* menuBar)
{
	T_ASSERT_M (!m_menuBar, L"Only a single menubar is allowed");
	m_menuBar = menuBar;
}

void FormWin32::unregisterMenuBar(MenuBarWin32* menuBar)
{
	T_ASSERT_M (m_menuBar == menuBar, L"Unregistered menubar");
	m_menuBar = 0;
}

#endif

LRESULT FormWin32::eventInitMenuPopup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
#if !defined(WINCE)

	LRESULT result;

	if (m_menuBar)
	{
		result = m_menuBar->eventInitMenuPopup(hWnd, message, wParam, lParam, pass);
		if (!pass)
			return result;
	}

#endif

	return 0;
}

LRESULT FormWin32::eventMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
#if !defined(WINCE)

	LRESULT result;

	if (m_menuBar)
	{
		result = m_menuBar->eventMenuCommand(hWnd, message, wParam, lParam, pass);
		if (!pass)
			return result;
	}

#endif

	return 0;
}

LRESULT FormWin32::eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	CloseEvent closeEvent(m_owner);
	m_owner->raiseEvent(&closeEvent);
	if (closeEvent.consumed() && closeEvent.cancelled())
		return TRUE;

	DestroyWindow(hWnd);
	return TRUE;
}

LRESULT FormWin32::eventDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	PostQuitMessage(0);
	return TRUE;
}

	}
}

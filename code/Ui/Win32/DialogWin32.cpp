#include "Ui/Dialog.h"
#include "Ui/Win32/DialogWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const UINT WM_ENDMODAL = WM_USER + 2000;

		}

DialogWin32::DialogWin32(EventSubject* owner)
:	WidgetWin32Impl< IDialog >(owner)
,	m_modal(false)
,	m_minSize(0, 0)
,	m_centerDesktop(false)
,	m_result(0)
{
}

bool DialogWin32::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
#if !defined(WINCE)
	DWORD nativeStyle = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

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

	if (style & Dialog::WsCenterDesktop)
		m_centerDesktop = true;
	else
		m_centerDesktop = false;

#else
	DWORD nativeStyle = WS_POPUP;
#endif

	HWND hWndParent = 0;
	if (parent)
		hWndParent = reinterpret_cast< HWND >(parent->getInternalHandle());

	if (!m_hWnd.create(
		hWndParent,
		_T("TraktorDialogWin32Class"),
		wstots(text).c_str(),
		nativeStyle,
		0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height
	))
		return false;

	if (!WidgetWin32Impl::create(0))
		return false;

	if (hWndParent)
	{
		MONITORINFO miParent, miDialog;

		miParent.cbSize = sizeof(miParent);
		GetMonitorInfo(MonitorFromWindow(hWndParent, MONITOR_DEFAULTTONEAREST), &miParent);

		miDialog.cbSize = sizeof(miDialog);
		GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &miDialog);

		RECT rcDialog;
		GetWindowRect(m_hWnd, &rcDialog);

		INT x = miParent.rcWork.left + rcDialog.left - miDialog.rcWork.left;
		INT y = miParent.rcWork.top + rcDialog.top - miDialog.rcWork.top;

		SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	m_hWnd.registerMessageHandler(WM_INITDIALOG, new MethodMessageHandler< DialogWin32 >(this, &DialogWin32::eventInitDialog));
#if !defined(WINCE)
	m_hWnd.registerMessageHandler(WM_SIZING, new MethodMessageHandler< DialogWin32 >(this, &DialogWin32::eventSizing));
#endif
	m_hWnd.registerMessageHandler(WM_CLOSE, new MethodMessageHandler< DialogWin32 >(this, &DialogWin32::eventClose));
	m_hWnd.registerMessageHandler(WM_ENDMODAL, new MethodMessageHandler< DialogWin32 >(this, &DialogWin32::eventEndModal));

	return true;
}

void DialogWin32::setIcon(drawing::Image* icon)
{
}

int DialogWin32::showModal()
{
	MSG msg;
	
	// Disable parent window, should be application main window.
	HWND hParentWnd = GetParent(m_hWnd);
	if (hParentWnd)
	{
		while (GetParent(hParentWnd))
			hParentWnd = GetParent(hParentWnd);
		EnableWindow(hParentWnd, FALSE);
	}

	HWND hCenterWnd = m_centerDesktop ? GetDesktopWindow() : hParentWnd;
	if (!hCenterWnd)
		hCenterWnd = GetDesktopWindow();

	// Place dialog window centered above parent window.
	RECT rcParent;
	GetWindowRect(hCenterWnd, &rcParent);
	POINT pntPos =
	{
		rcParent.left + ((rcParent.right - rcParent.left) - getRect().getWidth()) / 2,
		rcParent.top + ((rcParent.bottom - rcParent.top) - getRect().getHeight()) / 2
	};
	SetWindowPos(m_hWnd, HWND_TOP, pntPos.x, pntPos.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	
	// Handle events from the dialog.
	m_result = DrCancel;
	m_modal = true;

	while (m_modal)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!IsDialogMessage(m_hWnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
			Sleep(100);
	}

	if (hParentWnd)
	{
		// Enable parent window.
		EnableWindow(hParentWnd, TRUE);
		SetWindowPos(hParentWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	return m_result;
}

void DialogWin32::endModal(int result)
{
	T_ASSERT_M (m_modal, L"Not modal");
	SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
	PostMessage(m_hWnd, WM_ENDMODAL, result, 0);
}

void DialogWin32::setMinSize(const Size& minSize)
{
	m_minSize = minSize;
}

void DialogWin32::setVisible(bool visible)
{
	if (visible == isVisible(false))
		return;

	if (visible)
		ShowWindow(m_hWnd, SW_SHOW);
	else
		ShowWindow(m_hWnd, SW_HIDE);
}

LRESULT DialogWin32::eventInitDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	skip = false;
	return TRUE;
}

#if !defined(WINCE)

LRESULT DialogWin32::eventSizing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	LPRECT rc = reinterpret_cast< LPRECT >(lParam);
	
	int width = rc->right - rc->left;
	int height = rc->bottom - rc->top;

	width = std::max< int >(m_minSize.cx, width);
	height = std::max< int >(m_minSize.cy, height);

	// Top or bottom.
	switch (wParam)
	{
	case WMSZ_BOTTOM:
	case WMSZ_BOTTOMLEFT:
	case WMSZ_BOTTOMRIGHT:
		rc->bottom = rc->top + height;
		break;

	case WMSZ_TOP:
	case WMSZ_TOPLEFT:
	case WMSZ_TOPRIGHT:
		rc->top = rc->bottom - height;
		break;
	}

	// Left or right.
	switch (wParam)
	{
	case WMSZ_BOTTOMLEFT:
	case WMSZ_LEFT:
	case WMSZ_TOPLEFT:
		rc->left = rc->right - width;
		break;

	case WMSZ_BOTTOMRIGHT:
	case WMSZ_RIGHT:
	case WMSZ_TOPRIGHT:
		rc->right = rc->left + width;
		break;
	}

	return TRUE;
}

#endif

LRESULT DialogWin32::eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	skip = false;

	CloseEvent closeEvent(m_owner);
	m_owner->raiseEvent(&closeEvent);

	// Note: It's possible we've been destroyed atm thus it's not
	// really safe to assume "this" is valid.

	if (closeEvent.consumed())
		return 0;

	if (m_modal)
		endModal(DrCancel);

	return 0;
}

LRESULT DialogWin32::eventEndModal(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	m_modal = false;
	m_result = wParam;
	return 0;
}

	}
}

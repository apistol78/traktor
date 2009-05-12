#include "Ui/Win32/DialogWin32.h"
#include "Ui/Dialog.h"
#include "Ui/Events/CloseEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const UINT WM_ENDMODAL = WM_USER + 1;

		}

DialogWin32::DialogWin32(EventSubject* owner)
:	WidgetWin32Impl< IDialog >(owner)
,	m_modal(false)
,	m_minSize(0, 0)
,	m_centerDesktop(false)
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

	if (!m_hWnd.create(
		parent ? reinterpret_cast< HWND >(parent->getInternalHandle()) : 0,
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

	if (!WidgetWin32Impl::create(0))
		return false;

#if !defined(WINCE)
	m_hWnd.registerMessageHandler(WM_SIZING, new MethodMessageHandler< DialogWin32 >(this, &DialogWin32::eventSizing));
#endif
	m_hWnd.registerMessageHandler(WM_CLOSE, new MethodMessageHandler< DialogWin32 >(this, &DialogWin32::eventClose));

	return true;
}

void DialogWin32::setIcon(drawing::Image* icon)
{
}

int DialogWin32::showModal()
{
	int result = DrCancel;
	MSG msg;
	
	// Disable parent window, should be application main window.
	HWND hParentWnd = GetParent(m_hWnd);
	if (hParentWnd)
	{
		while(GetParent(hParentWnd))
			hParentWnd = GetParent(hParentWnd);
		EnableWindow(hParentWnd, FALSE);
	}

	HWND hCenterWnd = m_centerDesktop ? GetDesktopWindow() : hParentWnd;
	if (!hCenterWnd)
		hCenterWnd = GetDesktopWindow();

	// Position dialog window centered above parent window.
	RECT rcParent;
	GetWindowRect(hCenterWnd, &rcParent);
	POINT pntPos =
	{
		rcParent.left + ((rcParent.right - rcParent.left) - getRect().getWidth()) / 2,
		rcParent.top + ((rcParent.bottom - rcParent.top) - getRect().getHeight()) / 2
	};
	if(pntPos.x < 0)
		pntPos.x = 0;
	if(pntPos.y < 0)
		pntPos.y = 0;
	SetWindowPos(m_hWnd, NULL, pntPos.x, pntPos.y, 0, 0, SWP_NOSIZE);

	// Show dialog window.
	ShowWindow(m_hWnd, SW_SHOW);
	SetActiveWindow(m_hWnd);
	
	// Handle events from the dialog.
	m_modal = true;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(msg.message == WM_ENDMODAL)
		{
			result = int(msg.wParam);
			break;
		}
		else if(!IsDialogMessage(m_hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	m_modal = false;

	if (hParentWnd)
	{
		// Enable parent window.
		EnableWindow(hParentWnd, TRUE);
		SetForegroundWindow(hParentWnd);
	}

	// Hide dialog window.
	ShowWindow(m_hWnd, SW_HIDE);

	return result;
}

void DialogWin32::endModal(int result)
{
	T_ASSERT_M (m_modal, L"Not modal");
	PostMessage(NULL, WM_ENDMODAL, result, 0);
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
	{
		HWND hParentWnd = GetParent(m_hWnd);
		HWND hCenterWnd = m_centerDesktop ? GetDesktopWindow() : hParentWnd;
		if (!hCenterWnd)
			hCenterWnd = GetDesktopWindow();

		// Position dialog window centered above parent window.
		RECT rcParent;
		GetWindowRect(hCenterWnd, &rcParent);
		POINT pntPos =
		{
			rcParent.left + ((rcParent.right - rcParent.left) - getRect().getWidth()) / 2,
			rcParent.top + ((rcParent.bottom - rcParent.top) - getRect().getHeight()) / 2
		};
		if(pntPos.x < 0)
			pntPos.x = 0;
		if(pntPos.y < 0)
			pntPos.y = 0;
		SetWindowPos(m_hWnd, NULL, pntPos.x, pntPos.y, 0, 0, SWP_NOSIZE);
		ShowWindow(m_hWnd, SW_SHOW);
	}
	else
		ShowWindow(m_hWnd, SW_HIDE);
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
	if (m_modal)
	{
		endModal(DrCancel);
		skip = false;
	}
	else
	{
		CloseEvent closeEvent(m_owner, 0);
		m_owner->raiseEvent(EiClose, &closeEvent);
	}
	return TRUE;
}

	}
}

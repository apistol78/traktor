#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/StyleSheet.h"
#include "Ui/Events/ContentChangeEvent.h"
#include "Ui/Win32/EditWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int32_t c_heightMargin = 13;

COLORREF getColorRef(const Color4ub& c)
{
	return RGB(c.r, c.g, c.b);
}

		}

EditWin32::EditWin32(EventSubject* owner)
:	WidgetWin32Impl< IEdit >(owner)
,	m_borderColor(0, 0, 0, 0)
{
}

bool EditWin32::create(IWidget* parent, const std::wstring& text, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style & ~(WsBorder | WsClientBorder), nativeStyle, nativeStyleEx);

	if (style & Edit::WsReadOnly)
		nativeStyle |= ES_READONLY;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("EDIT"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_LEFT | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< EditWin32 >(this, &EditWin32::eventCommand));
	m_hWnd.registerMessageHandler(WM_REFLECTED_CTLCOLORSTATIC, new MethodMessageHandler< EditWin32 >(this, &EditWin32::eventCtlColorStatic));
	m_hWnd.registerMessageHandler(WM_REFLECTED_CTLCOLOREDIT, new MethodMessageHandler< EditWin32 >(this, &EditWin32::eventCtlColorEdit));

	if ((style & (WsBorder | WsClientBorder)) != 0)
	{
		m_hWnd.registerMessageHandler(WM_NCCALCSIZE, new MethodMessageHandler< EditWin32 >(this, &EditWin32::eventNonClientCalcSize));
		m_hWnd.registerMessageHandler(WM_NCPAINT, new MethodMessageHandler< EditWin32 >(this, &EditWin32::eventNonClientPaint));
	}

	return true;
}

void EditWin32::setSelection(int from, int to)
{
	m_hWnd.sendMessage(EM_SETSEL, (WPARAM)from, (LPARAM)to);
}

void EditWin32::getSelection(int& outFrom, int& outTo) const
{
	m_hWnd.sendMessage(EM_GETSEL, (WPARAM)(LPDWORD)&outFrom, (LPARAM)(LPDWORD)&outTo);
}

void EditWin32::selectAll()
{
	m_hWnd.sendMessage(EM_SETSEL, 0, -1);
}

void EditWin32::setBorderColor(const Color4ub& borderColor)
{
	m_borderColor = borderColor;
	SetWindowPos(m_hWnd, 0, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

Size EditWin32::getPreferedSize() const
{
	int32_t dpi = m_hWnd.getSystemDPI();
	int32_t currentFontHeight = getFont().getPixelSize();
	return Size((128 * dpi) / 96, currentFontHeight + c_heightMargin);
}

LRESULT EditWin32::eventCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	if (HIWORD(wParam) != EN_CHANGE)
		return 0;

	ContentChangeEvent contentChangeEvent(m_owner);
	m_owner->raiseEvent(&contentChangeEvent);

	return 0;
}

LRESULT EditWin32::eventCtlColorStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	if (!ss)
	{
		skip = true;
		return 0;
	}

	HDC hDC = (HDC)wParam;

	Color4ub color = ss->getColor(m_owner, L"color-disabled");
	SetTextColor(hDC, getColorRef(color));

	Color4ub backgroundColor = ss->getColor(m_owner, L"background-color-disabled");
	m_brushBackgroundDisabled = CreateSolidBrush(getColorRef(backgroundColor));

	SetBkColor(hDC, getColorRef(backgroundColor));

	return (LRESULT)m_brushBackgroundDisabled.getHandle();
}

LRESULT EditWin32::eventCtlColorEdit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	if (!ss)
	{
		skip = true;
		return 0;
	}

	HDC hDC = (HDC)wParam;

	Color4ub color = ss->getColor(m_owner, L"color");
	SetTextColor(hDC, getColorRef(color));

	Color4ub backgroundColor = ss->getColor(m_owner, L"background-color");
	m_brushBackground = CreateSolidBrush(getColorRef(backgroundColor));

	SetBkColor(hDC, getColorRef(backgroundColor));

	return (LRESULT)m_brushBackground.getHandle();
}

LRESULT EditWin32::eventNonClientCalcSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	if (!ss)
	{
		skip = true;
		return 0;
	}

	if (wParam == FALSE)
	{
		LPRECT lpClient = reinterpret_cast< LPRECT >(lParam);
		T_ASSERT (lpClient);

		lpClient->left++;
		lpClient->top++;
		lpClient->right--;
		lpClient->bottom--;
	}
	else
	{
		LPNCCALCSIZE_PARAMS lpParams = reinterpret_cast< LPNCCALCSIZE_PARAMS >(lParam);
		T_ASSERT (lpParams);

		lpParams->rgrc[0].left++;
		lpParams->rgrc[0].top++;
		lpParams->rgrc[0].right--;
		lpParams->rgrc[0].bottom--;
	}

	return 0;
}

LRESULT EditWin32::eventNonClientPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	if (!ss)
	{
		skip = true;
		return 0;
	}

	HDC hDC = GetWindowDC(hWnd);
	if (!hDC)
	{
		skip = true;
		return 0;
	}

	RECT rcWindow;
	GetWindowRect(hWnd, &rcWindow);
	OffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);

	SmartBrush borderBrush = CreateSolidBrush(getColorRef(
		m_borderColor.a != 0 ? m_borderColor : ss->getColor(m_owner, isEnable() ? L"border-color" : L"border-color-disabled")
	));
	HGDIOBJ hDefaultBrush = SelectObject(hDC, borderBrush);
	FrameRect(hDC, &rcWindow, borderBrush);
	SelectObject(hDC, hDefaultBrush);

	ReleaseDC(hWnd, hDC);
	return 0;
}

	}
}

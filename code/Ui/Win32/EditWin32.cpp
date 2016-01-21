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
{
}

bool EditWin32::create(IWidget* parent, const std::wstring& text, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (style & Edit::WsReadOnly)
		nativeStyle |= ES_READONLY;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("EDIT"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_LEFT/* | ES_WANTRETURN*/ | nativeStyle,
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

	}
}

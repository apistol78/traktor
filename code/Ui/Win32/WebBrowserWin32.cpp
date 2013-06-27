#include "Ui/WebBrowser.h"
#include "Ui/Win32/WebBrowserWin32.h"

namespace traktor
{
	namespace ui
	{

WebBrowserWin32::WebBrowserWin32(EventSubject* owner)
:	WidgetWin32Impl< IWebBrowser >(owner)
{
}

bool WebBrowserWin32::create(IWidget* parent, const std::wstring& url)
{
	/*
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

	if (style & WebBrowser::WsDefaultWebBrowser)
		nativeStyle |= BS_DEFPUSHBUTTON;
	else
		nativeStyle |= BS_PUSHBUTTON;

	if (style & WebBrowser::WsToggle)
		nativeStyle |= BS_AUTOCHECKBOX | BS_PUSHLIKE;

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		_T("BUTTON"),
		wstots(text).c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		s_buttonId,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(0))
		return false;

	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< WebBrowserWin32 >(this, &WebBrowserWin32::eventCommand));

	if (style & WebBrowser::WsDefaultWebBrowser)
	{
		SendMessage(
			(HWND)parent->getInternalHandle(),
			DM_SETDEFID,
			s_buttonId,
			0
		);
	}

	s_buttonId++;
	*/
	return true;
}

void WebBrowserWin32::navigate(const std::wstring& url)
{
}

	}
}

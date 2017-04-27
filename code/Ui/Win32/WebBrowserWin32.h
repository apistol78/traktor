/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_WebBrowserWin32_H
#define traktor_ui_WebBrowserWin32_H

#include <exdisp.h>
#include "Core/Misc/ComRef.h"
#include "Ui/Itf/IWebBrowser.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class WebBrowserWin32 : public WidgetWin32Impl< IWebBrowser >
{
public:
	WebBrowserWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& url);

	virtual void navigate(const std::wstring& url);

	virtual void forward();

	virtual void back();

	virtual void reload(bool forced);

	virtual bool ready() const;

private:
	ComRef< ::IOleClientSite > m_clientSite;
	ComRef< ::IStorage > m_storage;
	ComRef< ::IOleObject > m_webBrowserObject;
	ComRef< ::IWebBrowser2 > m_webBrowser;
	bool m_doneVerb;

	LRESULT eventSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& outPass);
};

	}
}

#endif	// traktor_ui_WebBrowserWin32_H

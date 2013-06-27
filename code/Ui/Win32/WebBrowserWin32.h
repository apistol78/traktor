#ifndef traktor_ui_WebBrowserWin32_H
#define traktor_ui_WebBrowserWin32_H

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
};

	}
}

#endif	// traktor_ui_WebBrowserWin32_H

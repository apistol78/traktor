#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/WebBrowser.h"
#include "Ui/Itf/IWebBrowser.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.WebBrowser", WebBrowser, Widget)

bool WebBrowser::create(Widget* parent, const std::wstring& url)
{
	if (!parent)
		return false;

	IWebBrowser* webBrowser = Application::getInstance()->getWidgetFactory()->createWebBrowser(this);
	if (!webBrowser)
	{
		log::error << L"Failed to create native widget peer (WebBrowser)" << Endl;
		return false;
	}

	if (!webBrowser->create(parent->getIWidget(), url))
	{
		webBrowser->destroy();
		return false;
	}

	m_widget = webBrowser;

	return Widget::create(parent);
}

void WebBrowser::navigate(const std::wstring& url)
{
	T_ASSERT (m_widget);
	static_cast< IWebBrowser* >(m_widget)->navigate(url);
}

void WebBrowser::forward()
{
	T_ASSERT (m_widget);
	static_cast< IWebBrowser* >(m_widget)->forward();
}

void WebBrowser::back()
{
	T_ASSERT (m_widget);
	static_cast< IWebBrowser* >(m_widget)->back();
}

	}
}

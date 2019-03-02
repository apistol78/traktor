#pragma once

#include "Net/Url.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class ToolBarButtonClickEvent;
class WebBrowser;

	}

	namespace editor
	{

class IEditor;

/*! \brief
 * \ingroup Editor
 */
class WebBrowserPage : public ui::Container
{
	T_RTTI_CLASS;

public:
	WebBrowserPage(IEditor* editor);

	bool create(ui::Widget* parent, const net::Url& url);

private:
	IEditor* m_editor;
	Ref< ui::WebBrowser > m_browser;

	void eventToolClick(ui::ToolBarButtonClickEvent* event);
};

	}
}


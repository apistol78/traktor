#ifndef traktor_editor_WebBrowserPage_H
#define traktor_editor_WebBrowserPage_H

#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

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

	bool create(ui::Widget* parent);

private:
	IEditor* m_editor;
	Ref< ui::WebBrowser > m_browser;

	void eventToolClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_WebBrowserPage_H

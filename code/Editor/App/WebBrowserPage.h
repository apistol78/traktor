/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_WebBrowserPage_H
#define traktor_editor_WebBrowserPage_H

#include "Net/Url.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class WebBrowser;

		namespace custom
		{

class ToolBarButtonClickEvent;

		}
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

	void eventToolClick(ui::custom::ToolBarButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_WebBrowserPage_H

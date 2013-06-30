#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/IEditor.h"
#include "Editor/App/WebBrowserPage.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/WebBrowser.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.WebBrowserPage", WebBrowserPage, ui::Container)

WebBrowserPage::WebBrowserPage(IEditor* editor)
:	m_editor(editor)
{
}

bool WebBrowserPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	std::wstring url = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.HomeUrl", L"about:blank");

	Ref< ui::custom::ToolBar > toolbar = new ui::custom::ToolBar();
	toolbar->create(this);
	toolbar->addItem(new ui::custom::ToolBarButton(L"Home", ui::Command(L"Editor.WebBrowser.Home")));
	toolbar->addItem(new ui::custom::ToolBarButton(L"Back", ui::Command(L"Editor.WebBrowser.Back")));
	toolbar->addItem(new ui::custom::ToolBarButton(L"Forward", ui::Command(L"Editor.WebBrowser.Forward")));
	toolbar->addClickEventHandler(ui::createMethodHandler(this, &WebBrowserPage::eventToolClick));

	m_browser = new ui::WebBrowser();
	m_browser->create(this, url);

	return true;
}

void WebBrowserPage::eventToolClick(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent* >(event);
	if (cmdEvent->getCommand() == L"Editor.WebBrowser.Home")
	{
		std::wstring url = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.HomeUrl", L"about:blank");
		m_browser->navigate(url);
	}
	else if (cmdEvent->getCommand() == L"Editor.WebBrowser.Back")
		m_browser->back();
	else if (cmdEvent->getCommand() == L"Editor.WebBrowser.Forward")
		m_browser->forward();
}

	}
}

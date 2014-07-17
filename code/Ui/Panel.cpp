#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Panel.h"
#include "Ui/Itf/IPanel.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Panel", Panel, Container)

bool Panel::create(Widget* parent, const std::wstring& text, Layout* layout)
{
	if (!parent)
		return false;

	IPanel* panel = Application::getInstance()->getWidgetFactory()->createPanel(this);
	if (!panel)
	{
		log::error << L"Failed to create native widget peer (Panel)" << Endl;
		return false;
	}

	Ref< Layout > refLayout = layout;

	if (!panel->create(parent->getIWidget(), text))
	{
		panel->destroy();
		return false;
	}

	return Container::create(parent, WsNone, refLayout);
}

	}
}

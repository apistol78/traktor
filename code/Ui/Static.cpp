#include "Ui/Static.h"
#include "Ui/Application.h"
#include "Ui/Itf/IStatic.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Static", Static, Widget)

bool Static::create(Widget* parent, const std::wstring& text)
{
	if (!parent)
		return false;

	IStatic* staticText = Application::getInstance().getWidgetFactory()->createStatic(this);
	if (!staticText)
	{
		log::error << L"Failed to create native widget peer (Static)" << Endl;
		return false;
	}

	if (!staticText->create(parent->getIWidget(), text))
	{
		staticText->destroy();
		return false;
	}

	m_widget = staticText;

	return Widget::create(parent);
}

	}
}

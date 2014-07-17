#include "Ui/Button.h"
#include "Ui/Application.h"
#include "Ui/Itf/IButton.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Button", Button, Widget)

bool Button::create(Widget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;

	IButton* button = Application::getInstance()->getWidgetFactory()->createButton(this);
	if (!button)
	{
		log::error << L"Failed to create native widget peer (Button)" << Endl;
		return false;
	}

	if (!button->create(parent->getIWidget(), text, style))
	{
		button->destroy();
		return false;
	}

	m_widget = button;

	return Widget::create(parent);
}

void Button::setState(bool state)
{
	T_ASSERT (m_widget);
	static_cast< IButton* >(m_widget)->setState(state);
}

bool Button::getState() const
{
	T_ASSERT (m_widget);
	return static_cast< IButton* >(m_widget)->getState();
}

	}
}

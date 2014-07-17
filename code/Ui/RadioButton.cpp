#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/RadioButton.h"
#include "Ui/Itf/IRadioButton.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.RadioButton", RadioButton, Widget)

bool RadioButton::create(Widget* parent, const std::wstring& text, bool checked)
{
	if (!parent)
		return false;

	IRadioButton* radioButton = Application::getInstance()->getWidgetFactory()->createRadioButton(this);
	if (!radioButton)
	{
		log::error << L"Failed to create native widget peer (RadioButton)" << Endl;
		return false;
	}

	if (!radioButton->create(parent->getIWidget(), text, checked))
	{
		radioButton->destroy();
		return false;
	}

	m_widget = radioButton;

	return Widget::create(parent);
}

void RadioButton::setChecked(bool checked)
{
	static_cast< IRadioButton* >(m_widget)->setChecked(checked);
}

bool RadioButton::isChecked() const
{
	return static_cast< IRadioButton* >(m_widget)->isChecked();
}

	}
}

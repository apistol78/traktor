#include "Ui/CheckBox.h"
#include "Ui/Application.h"
#include "Ui/Itf/ICheckBox.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CheckBox", CheckBox, Widget)

bool CheckBox::create(Widget* parent, const std::wstring& text, bool checked)
{
	if (!parent)
		return false;

	ICheckBox* checkBox = Application::getInstance()->getWidgetFactory()->createCheckBox(this);
	if (!checkBox)
	{
		log::error << L"Failed to create native widget peer (CheckBox)" << Endl;
		return false;
	}

	if (!checkBox->create(parent->getIWidget(), text, checked))
	{
		checkBox->destroy();
		return false;
	}

	m_widget = checkBox;

	return Widget::create(parent);
}

void CheckBox::setChecked(bool checked)
{
	return static_cast< ICheckBox* >(m_widget)->setChecked(checked);
}

bool CheckBox::isChecked() const
{
	return static_cast< ICheckBox* >(m_widget)->isChecked();
}

	}
}

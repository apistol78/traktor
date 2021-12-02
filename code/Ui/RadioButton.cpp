#include "Ui/RadioButton.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.RadioButton", RadioButton, Widget)

bool RadioButton::create(Widget* parent, const std::wstring& text, bool checked)
{
	if (!Widget::create(parent))
		return false;

	return true;
}

void RadioButton::setChecked(bool checked)
{
	//static_cast< IRadioButton* >(m_widget)->setChecked(checked);
}

bool RadioButton::isChecked() const
{
	//return static_cast< IRadioButton* >(m_widget)->isChecked();
	return false;
}

	}
}

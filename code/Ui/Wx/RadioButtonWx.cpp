#include "Ui/Wx/RadioButtonWx.h"

namespace traktor
{
	namespace ui
	{

RadioButtonWx::RadioButtonWx(EventSubject* owner)
:	WidgetWxImpl< IRadioButton, wxRadioButton >(owner)
{
}

bool RadioButtonWx::create(IWidget* parent, const std::wstring& text, bool checked)
{
	m_window = new wxRadioButton();

	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1,
		wstots(text).c_str()
	))
	{
		m_window->Destroy();
		return false;
	}
	
	if (!WidgetWxImpl< IRadioButton, wxRadioButton >::create(0))
		return false;

	return true;
}

void RadioButtonWx::setChecked(bool checked)
{
	static_cast< wxRadioButton* >(m_window)->SetValue(checked);
}

bool RadioButtonWx::isChecked() const
{
	return static_cast< wxRadioButton* >(m_window)->GetValue();
}

	}
}

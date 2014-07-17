#include "Ui/ComboBox.h"
#include "Ui/Application.h"
#include "Ui/Itf/IComboBox.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ComboBox", ComboBox, Widget)

bool ComboBox::create(Widget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;

	IComboBox* comboBox = Application::getInstance()->getWidgetFactory()->createComboBox(this);
	if (!comboBox)
	{
		log::error << L"Failed to create native widget peer (ComboBox)" << Endl;
		return false;
	}

	if (!comboBox->create(parent->getIWidget(), text, style))
	{
		comboBox->destroy();
		return false;
	}

	m_widget = comboBox;

	return Widget::create(parent);
}

int ComboBox::add(const std::wstring& item)
{
	T_ASSERT (m_widget);
	return static_cast< IComboBox* >(m_widget)->add(item.c_str());
}

bool ComboBox::remove(int index)
{
	T_ASSERT (m_widget);
	return static_cast< IComboBox* >(m_widget)->remove(index);
}

void ComboBox::removeAll()
{
	T_ASSERT (m_widget);
	static_cast< IComboBox* >(m_widget)->removeAll();
}

int ComboBox::count() const
{
	T_ASSERT (m_widget);
	return static_cast< IComboBox* >(m_widget)->count();
}

std::wstring ComboBox::get(int index) const
{
	T_ASSERT (m_widget);
	return static_cast< IComboBox* >(m_widget)->get(index);
}

void ComboBox::select(int index)
{
	T_ASSERT (m_widget);
	static_cast< IComboBox* >(m_widget)->select(index);
}

int ComboBox::getSelected() const
{
	T_ASSERT (m_widget);
	return static_cast< IComboBox* >(m_widget)->getSelected();
}

std::wstring ComboBox::getSelectedItem() const
{
	T_ASSERT (m_widget);
	int index = getSelected();
	return (index >= 0) ? get(index) : std::wstring();
}

	}
}

#include "Ui/DropDown.h"
#include "Ui/Application.h"
#include "Ui/Itf/IDropDown.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DropDown", DropDown, Widget)

bool DropDown::create(Widget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;

	IDropDown* dropDown = Application::getInstance().getWidgetFactory()->createDropDown(this);
	if (!dropDown)
	{
		log::error << L"Failed to create native widget peer (DropDown)" << Endl;
		return false;
	}

	if (!dropDown->create(parent->getIWidget(), text, style))
	{
		dropDown->destroy();
		return false;
	}

	m_widget = dropDown;

	return Widget::create(parent);
}

int DropDown::add(const std::wstring& item)
{
	T_ASSERT (m_widget);
	return static_cast< IDropDown* >(m_widget)->add(item.c_str());
}

bool DropDown::remove(int index)
{
	T_ASSERT (m_widget);
	return static_cast< IDropDown* >(m_widget)->remove(index);
}

void DropDown::removeAll()
{
	T_ASSERT (m_widget);
	static_cast< IDropDown* >(m_widget)->removeAll();
}

int DropDown::count() const
{
	T_ASSERT (m_widget);
	return static_cast< IDropDown* >(m_widget)->count();
}

std::wstring DropDown::get(int index) const
{
	T_ASSERT (m_widget);
	return static_cast< IDropDown* >(m_widget)->get(index);
}

void DropDown::select(int index)
{
	T_ASSERT (m_widget);
	static_cast< IDropDown* >(m_widget)->select(index);
}

int DropDown::getSelected() const
{
	T_ASSERT (m_widget);
	return static_cast< IDropDown* >(m_widget)->getSelected();
}

std::wstring DropDown::getSelectedItem() const
{
	T_ASSERT (m_widget);
	int index = getSelected();
	return (index >= 0) ? get(index) : std::wstring();
}

void DropDown::addSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSelectionChange, eventHandler);
}

	}
}

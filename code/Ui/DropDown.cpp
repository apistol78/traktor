#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/DropDown.h"
#include "Ui/Itf/IDropDown.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DropDown", DropDown, Widget)

bool DropDown::create(Widget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;

	IDropDown* dropDown = Application::getInstance()->getWidgetFactory()->createDropDown(this);
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

int DropDown::add(const std::wstring& item, Object* data)
{
	T_ASSERT (m_widget);

	int index = static_cast< IDropDown* >(m_widget)->add(item.c_str());
	if (index >= 0)
		m_data[index] = data;

	return index;
}

bool DropDown::remove(int index)
{
	T_ASSERT (m_widget);

	if (!static_cast< IDropDown* >(m_widget)->remove(index))
		return false;

	m_data[index] = 0;
	return true;
}

void DropDown::removeAll()
{
	T_ASSERT (m_widget);
	static_cast< IDropDown* >(m_widget)->removeAll();
	m_data.clear();
}

int DropDown::count() const
{
	T_ASSERT (m_widget);
	return static_cast< IDropDown* >(m_widget)->count();
}

void DropDown::setItem(int index, const std::wstring& item)
{
	T_ASSERT (m_widget);
	static_cast< IDropDown* >(m_widget)->set(index, item);
}

void DropDown::setData(int index, Object* data)
{
	T_ASSERT (m_widget);
	m_data[index] = data;
}

std::wstring DropDown::getItem(int index) const
{
	T_ASSERT (m_widget);
	return static_cast< IDropDown* >(m_widget)->get(index);
}

Ref< Object > DropDown::getData(int index) const
{
	std::map< int, Ref< Object > >::const_iterator i = m_data.find(index);
	return (i != m_data.end()) ? i->second.ptr() : 0;
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
	return index >= 0 ? getItem(index) : std::wstring();
}

Ref< Object > DropDown::getSelectedData() const
{
	T_ASSERT (m_widget);
	int index = getSelected();
	return index >= 0 ? getData(index) : 0;
}

	}
}

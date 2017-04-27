/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/ListBox.h"
#include "Ui/Itf/IListBox.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListBox", ListBox, Widget)

bool ListBox::create(Widget* parent, const std::wstring& text, int style)
{
	if (!parent)
		return false;

	IListBox* listBox = Application::getInstance()->getWidgetFactory()->createListBox(this);
	if (!listBox)
	{
		log::error << L"Failed to create native widget peer (ListBox)" << Endl;
		return false;
	}

	if (!listBox->create(parent->getIWidget(), style))
	{
		listBox->destroy();
		return false;
	}

	m_widget = listBox;

	return Widget::create(parent);
}

Size ListBox::getPreferedSize() const
{
	T_ASSERT (m_widget);
	return Size(m_widget->getPreferedSize().cx, 100);
}

int ListBox::add(const std::wstring& item, Object* data)
{
	T_ASSERT (m_widget);
	
	int index = static_cast< IListBox* >(m_widget)->add(item);
	if (index >= 0)
		m_data[index] = data;

	return index;
}

bool ListBox::remove(int index)
{
	T_ASSERT (m_widget);

	if (!static_cast< IListBox* >(m_widget)->remove(index))
		return false;

	m_data[index] = 0;
	return true;
}

void ListBox::removeAll()
{
	T_ASSERT (m_widget);
	static_cast< IListBox* >(m_widget)->removeAll();
	m_data.clear();
}

int ListBox::count() const
{
	T_ASSERT (m_widget);
	return static_cast< IListBox* >(m_widget)->count();
}

void ListBox::setItem(int index, const std::wstring& item)
{
	T_ASSERT (m_widget);
	static_cast< IListBox* >(m_widget)->set(index, item);
}

void ListBox::setData(int index, Object* data)
{
	T_ASSERT (m_widget);
	m_data[index] = data;
}

std::wstring ListBox::getItem(int index) const
{
	T_ASSERT (m_widget);
	return static_cast< IListBox* >(m_widget)->get(index);
}

Ref< Object > ListBox::getData(int index) const
{
	std::map< int, Ref< Object > >::const_iterator i = m_data.find(index);
	return (i != m_data.end()) ? i->second.ptr() : 0;
}

void ListBox::select(int index)
{
	T_ASSERT (m_widget);
	static_cast< IListBox* >(m_widget)->select(index);
}

bool ListBox::selected(int index) const
{
	T_ASSERT (m_widget);
	return static_cast< IListBox* >(m_widget)->selected(index);
}

int ListBox::getSelected(std::vector< int >& selected) const
{
	T_ASSERT (m_widget);

	for (int i = 0; i < count(); ++i)
	{
		if (this->selected(i))
			selected.push_back(i);
	}

	return int(selected.size());
}

int ListBox::getSelected() const
{
	T_ASSERT (m_widget);

	for (int i = 0; i < count(); ++i)
	{
		if (selected(i))
			return i;
	}

	return -1;
}

std::wstring ListBox::getSelectedItem() const
{
	T_ASSERT (m_widget);
	return getItem(getSelected());
}

Ref< Object > ListBox::getSelectedData() const
{
	T_ASSERT (m_widget);
	return getData(getSelected());
}

int ListBox::getItemHeight() const
{
	T_ASSERT (m_widget);
	return getItemRect(0).getHeight();
}

Rect ListBox::getItemRect(int index) const
{
	T_ASSERT (m_widget);
	return static_cast< IListBox* >(m_widget)->getItemRect(index);
}

	}
}

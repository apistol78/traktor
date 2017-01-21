#include "Ui/Application.h"
#include "Ui/Custom/ListBox/ListBox.h"
#include "Ui/Custom/ListBox/ListBoxItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
		
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ListBox", ListBox, AutoWidget)

ListBox::ListBox()
:	m_style(0)
{
}

bool ListBox::create(Widget* parent, int32_t style)
{
	if (!AutoWidget::create(parent, style | WsDoubleBuffer))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ListBox::eventButtonDown);

	m_style = style;
	return true;
}

Size ListBox::getPreferedSize() const
{
	return Size(100, 100);
}

int32_t ListBox::add(const std::wstring& item, Object* data)
{
	Ref< ListBoxItem > lbi = new ListBoxItem();
	lbi->setText(item);
	lbi->setData(L"DATA", data);
	m_items.push_back(lbi);
	requestUpdate();
	return int32_t(m_items.size() - 1);
}

bool ListBox::remove(int32_t index)
{
	if (index >= 0 && index < m_items.size())
	{
		m_items.erase(m_items.begin() + index);
		requestUpdate();
		return true;
	}
	else
		return false;
}

void ListBox::removeAll()
{
	m_items.clear();
	requestUpdate();
}

int32_t ListBox::count() const
{
	return int32_t(m_items.size());
}

void ListBox::setItem(int32_t index, const std::wstring& item)
{
	if (index >= 0 && index < int32_t(m_items.size()))
	{
		m_items[index]->setText(item);
		requestUpdate();
	}
}

void ListBox::setData(int32_t index, Object* data)
{
	if (index >= 0 && index < int32_t(m_items.size()))
		m_items[index]->setData(L"DATA", data);
}

std::wstring ListBox::getItem(int32_t index) const
{
	if (index >= 0 && index < int32_t(m_items.size()))
		return m_items[index]->getText();
	else
		return L"";
}

Ref< Object > ListBox::getData(int32_t index) const
{
	if (index >= 0 && index < int32_t(m_items.size()))
		return m_items[index]->getData(L"DATA");
	else
		return 0;
}

void ListBox::select(int32_t index)
{
	if (index >= 0 && index < int32_t(m_items.size()))
		m_items[index]->setSelected(true);
}

bool ListBox::selected(int32_t index) const
{
	if (index >= 0 && index < int32_t(m_items.size()))
		return m_items[index]->isSelected();
	else
		return false;
}

int32_t ListBox::getSelected(std::vector< int32_t >& selected) const
{
	selected.resize(0);
	for (int32_t i = 0; i < int32_t(m_items.size()); ++i)
	{
		if (m_items[i]->isSelected())
			selected.push_back(i);
	}
	return int32_t(selected.size());
}

int32_t ListBox::getSelected() const
{
	for (int32_t i = 0; i < int32_t(m_items.size()); ++i)
	{
		if (m_items[i]->isSelected())
			return i;
	}
	return -1;
}

std::wstring ListBox::getSelectedItem() const
{
	int32_t index = getSelected();
	if (index >= 0)
		return m_items[index]->getText();
	else
		return L"";
}

Ref< Object > ListBox::getSelectedData() const
{
	int32_t index = getSelected();
	if (index >= 0)
		return m_items[index]->getData(L"DATA");
	else
		return 0;
}

int32_t ListBox::getItemHeight()
{
	return getFont().getPixelSize() + scaleBySystemDPI(6);
}

Rect ListBox::getItemRect(int32_t index) const
{
	if (index >= 0 && index < int32_t(m_items.size()))
		return getCellRect(m_items[index]);
	else
		return Rect();
}

void ListBox::eventButtonDown(MouseButtonDownEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	bool modified = false;

	if ((m_style & (WsMultiple | WsExtended)) == 0)
	{
		Ref< ListBoxItem > hitItem = dynamic_type_cast< ListBoxItem* >(hitTest(event->getPosition()));
		for (RefArray< ListBoxItem >::iterator i = m_items.begin(); i != m_items.end(); ++i)
			modified |= (*i)->setSelected(hitItem == *i);
	}
	else
	{
		Ref< ListBoxItem > hitItem = dynamic_type_cast< ListBoxItem* >(hitTest(event->getPosition()));
		if (hitItem)
			modified |= hitItem->setSelected(!hitItem->isSelected());
	}

	if (modified)
	{
		SelectionChangeEvent selectionChangeEvent(this);
		this->raiseEvent(&selectionChangeEvent);
	}
}

void ListBox::layoutCells(const Rect& rc)
{
	int32_t height = getItemHeight();

	Rect rcItem(rc.left, rc.top, rc.right, rc.top + height);
	for (uint32_t i = 0; i < m_items.size(); ++i)
	{
		placeCell(m_items[i], rcItem);
		rcItem = rcItem.offset(0, height);
	}
}

		}
	}
}
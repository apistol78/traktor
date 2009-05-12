#include <algorithm>
#include "Ui/ListViewItems.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListViewItems", ListViewItems, Object)

void ListViewItems::add(ListViewItem* item)
{
	m_items.push_back(item);
}

void ListViewItems::remove(ListViewItem* item)
{
	RefArray< ListViewItem >::iterator pos;
	if ((pos = std::find(m_items.begin(), m_items.end(), item)) == m_items.end())
		return;

	m_items.erase(pos);
}

int ListViewItems::count() const
{
	return int(m_items.size());
}

ListViewItem* ListViewItems::get(int index) const
{
	if (index < 0 || index >= int(m_items.size()))
		return 0;

	return m_items[index];
}

	}
}

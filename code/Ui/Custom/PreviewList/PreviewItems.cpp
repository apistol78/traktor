#include "Ui/Custom/PreviewList/PreviewItems.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PreviewItems", PreviewItems, Object)

void PreviewItems::add(PreviewItem* item)
{
	m_items.push_back(item);
}

void PreviewItems::remove(PreviewItem* item)
{
	RefArray< PreviewItem >::iterator pos;
	if ((pos = std::find(m_items.begin(), m_items.end(), item)) == m_items.end())
		return;

	m_items.erase(pos);
}

void PreviewItems::removeAll()
{
	m_items.resize(0);
}

int PreviewItems::count() const
{
	return int(m_items.size());
}

Ref< PreviewItem > PreviewItems::get(int index) const
{
	if (index < 0 || index >= int(m_items.size()))
		return 0;

	return m_items[index];
}

		}
	}
}

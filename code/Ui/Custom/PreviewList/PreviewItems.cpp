#include <algorithm>
#include "Ui/Custom/PreviewList/PreviewItem.h"
#include "Ui/Custom/PreviewList/PreviewItems.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{
                
struct ItemSortPred
{
	bool operator () (const PreviewItem* item1, const PreviewItem* item2) const
	{
		return item1->getText().compare(item2->getText()) < 0;
	}
};

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PreviewItems", PreviewItems, Object)

void PreviewItems::add(PreviewItem* item)
{
	m_items.push_back(item);
#if !defined(__APPLE__) && !defined(__PNACL__)
	std::sort(m_items.begin(), m_items.end(), ItemSortPred());
#endif
}

void PreviewItems::remove(PreviewItem* item)
{
	RefArray< PreviewItem >::iterator pos;
	if ((pos = std::find(m_items.begin(), m_items.end(), item)) != m_items.end())
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

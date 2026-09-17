/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PreviewList/PreviewItems.h"

#include "Ui/PreviewList/PreviewItem.h"

namespace traktor::ui
{
namespace
{

struct ItemSortPred
{
	bool operator()(const PreviewItem* item1, const PreviewItem* item2) const
	{
		return item1->getText().compare(item2->getText()) < 0;
	}
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewItems", PreviewItems, Object)

void PreviewItems::add(PreviewItem* item)
{
	m_items.push_back(item);
	m_items.sort(ItemSortPred());
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

int32_t PreviewItems::count() const
{
	return (int32_t)m_items.size();
}

PreviewItem* PreviewItems::get(int32_t index) const
{
	if (index >= 0 && index < (int32_t)m_items.size())
		return m_items[index];
	else
		return nullptr;
}

int32_t PreviewItems::indexOf(const PreviewItem* item) const
{
	auto pos = std::find(m_items.begin(), m_items.end(), item);
	if (pos != m_items.end())
		return (int32_t)std::distance(m_items.begin(), pos);
	else
		return -1;
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"

namespace traktor
{
	namespace ui
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

int PreviewItems::count() const
{
	return int(m_items.size());
}

Ref< PreviewItem > PreviewItems::get(int index) const
{
	if (index >= 0 && index < int(m_items.size()))
		return m_items[index];
	else
		return 0;
}

	}
}

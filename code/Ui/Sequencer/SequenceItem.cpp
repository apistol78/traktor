/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Sequencer/SequenceItem.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SequenceItem", SequenceItem, Object)

SequenceItem::SequenceItem(const std::wstring& name)
:	m_name(name)
,	m_selected(false)
{
}

void SequenceItem::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& SequenceItem::getName() const
{
	return m_name;
}

bool SequenceItem::setSelected(bool selected)
{
	if (m_selected == selected)
		return false;

	m_selected = selected;
	return true;
}

bool SequenceItem::isSelected() const
{
	return m_selected;
}

SequenceItem* SequenceItem::getParentItem() const
{
	return m_parent;
}

void SequenceItem::addChildItem(SequenceItem* childItem)
{
	T_ASSERT_M (!childItem->m_parent, L"Item already has a parent");
	childItem->m_parent = this;
	m_childItems.push_back(childItem);
}

void SequenceItem::addChildItemBefore(SequenceItem* beforeChildItem, SequenceItem* childItem)
{
	T_ASSERT_M (!childItem->m_parent, L"Item already has a parent");
	childItem->m_parent = this;

	RefArray< SequenceItem >::iterator i = std::find(m_childItems.begin(), m_childItems.end(), childItem);
	T_ASSERT(i != m_childItems.end());

	m_childItems.insert(i, childItem);
}

void SequenceItem::removeChildItem(SequenceItem* childItem)
{
	T_ASSERT_M (childItem->m_parent == this, L"Item not a child to this item");
	childItem->m_parent = nullptr;
	m_childItems.remove(childItem);
}

RefArray< SequenceItem >& SequenceItem::getChildItems()
{
	return m_childItems;
}

int32_t SequenceItem::getDepth() const
{
	int32_t depth = 0;
	for (const SequenceItem* i = this; i->getParentItem(); i = i->getParentItem())
		++depth;
	return depth;
}

}

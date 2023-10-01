/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "SolutionBuilder/ProjectItem.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"ProjectItem", ProjectItem, ISerializable)

void ProjectItem::addItem(ProjectItem* item)
{
	m_items.push_back(item);
}

void ProjectItem::removeItem(ProjectItem* item)
{
	m_items.remove(item);
}

void ProjectItem::setItems(const RefArray< ProjectItem >& items)
{
	m_items = items;
}

const RefArray< ProjectItem >& ProjectItem::getItems() const
{
	return m_items;
}

void ProjectItem::serialize(ISerializer& s)
{
	s >> MemberRefArray< ProjectItem >(L"items", m_items);
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ToolBar/ToolBarItem.h"
#include "Ui/ToolBar/ToolBarItemGroup.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarItemGroup", ToolBarItemGroup, Object)

ToolBarItem* ToolBarItemGroup::addItem(ToolBarItem* item)
{
	m_items.push_back(item);
	return item;
}

void ToolBarItemGroup::removeItem(ToolBarItem* item)
{
	m_items.remove(item);
}

void ToolBarItemGroup::setEnable(bool enable)
{
	for (auto item : m_items)
		item->setEnable(enable);
}

}

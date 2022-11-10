/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ToolBar/ToolBarItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarItem", ToolBarItem, Object)

ToolBarItem::ToolBarItem()
:	m_enable(true)
{
}

void ToolBarItem::setEnable(bool enable)
{
	m_enable = enable;
}

bool ToolBarItem::isEnable() const
{
	return m_enable;
}

	}
}

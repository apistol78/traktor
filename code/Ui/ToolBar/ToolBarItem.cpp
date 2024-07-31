/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ToolBar/ToolBarItem.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarItem", ToolBarItem, Object)

void ToolBarItem::setEnable(bool enable)
{
	m_enable = enable;
}

bool ToolBarItem::isEnable() const
{
	return m_enable;
}

}

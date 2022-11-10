/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PropertyList/PropertySelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PropertySelectionChangeEvent", PropertySelectionChangeEvent, SelectionChangeEvent)

PropertySelectionChangeEvent::PropertySelectionChangeEvent(EventSubject* sender, PropertyItem* item, int32_t index)
:	SelectionChangeEvent(sender)
,	m_item(item)
,	m_index(index)
{
}

PropertyItem* PropertySelectionChangeEvent::getItem() const
{
	return m_item;
}

int32_t PropertySelectionChangeEvent::getIndex() const
{
	return m_index;
}

	}
}

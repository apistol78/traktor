/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/GridView/GridRowDoubleClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridRowDoubleClickEvent", GridRowDoubleClickEvent, Event)

GridRowDoubleClickEvent::GridRowDoubleClickEvent(EventSubject* sender, GridRow* row, int32_t columnIndex)
:	Event(sender)
,	m_row(row)
,	m_columnIndex(columnIndex)
{
}

GridRow* GridRowDoubleClickEvent::getRow() const
{
	return m_row;
}

int32_t GridRowDoubleClickEvent::getColumnIndex() const
{
	return m_columnIndex;
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridRowMouseButtonDownEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridRowMouseButtonDownEvent", GridRowMouseButtonDownEvent, Event)

GridRowMouseButtonDownEvent::GridRowMouseButtonDownEvent(EventSubject* sender, GridRow* row, int32_t button, const Point& position)
:	Event(sender)
,	m_row(row)
,	m_button(button)
,	m_position(position)
{
}

GridRow* GridRowMouseButtonDownEvent::getRow() const
{
	return m_row;
}

int32_t GridRowMouseButtonDownEvent::getButton() const
{
	return m_button;
}

const Point& GridRowMouseButtonDownEvent::getPosition() const
{
	return m_position;
}

}

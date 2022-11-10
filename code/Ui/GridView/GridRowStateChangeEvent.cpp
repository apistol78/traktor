/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/GridView/GridRowStateChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridRowStateChangeEvent", GridRowStateChangeEvent, Event)

GridRowStateChangeEvent::GridRowStateChangeEvent(EventSubject* sender, GridRow* row)
:	Event(sender)
,	m_row(row)
{
}

GridRow* GridRowStateChangeEvent::getRow() const
{
	return m_row;
}

	}
}

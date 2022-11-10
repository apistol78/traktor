/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PreviewList/PreviewSelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewSelectionChangeEvent", PreviewSelectionChangeEvent, SelectionChangeEvent)

PreviewSelectionChangeEvent::PreviewSelectionChangeEvent(EventSubject* sender, PreviewItem* item)
:	SelectionChangeEvent(sender)
,	m_item(item)
{
}

PreviewItem* PreviewSelectionChangeEvent::getItem() const
{
	return m_item;
}

	}
}

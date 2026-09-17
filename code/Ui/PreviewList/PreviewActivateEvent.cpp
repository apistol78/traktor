/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PreviewList/PreviewActivateEvent.h"

#include "Ui/PreviewList/PreviewItem.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewActivateEvent", PreviewActivateEvent, Event)

PreviewActivateEvent::PreviewActivateEvent(EventSubject* sender, PreviewItem* item)
	: Event(sender)
	, m_item(item)
{
}

PreviewItem* PreviewActivateEvent::getItem() const
{
	return m_item;
}

}

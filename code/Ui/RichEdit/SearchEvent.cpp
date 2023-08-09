/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/RichEdit/SearchEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SearchEvent", SearchEvent, ui::Event)

SearchEvent::SearchEvent(
	ui::EventSubject* sender,
	bool preview
)
:	ui::Event(sender)
,	m_preview(preview)
{
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Envelope/EnvelopeContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EnvelopeContentChangeEvent", EnvelopeContentChangeEvent, ContentChangeEvent)

EnvelopeContentChangeEvent::EnvelopeContentChangeEvent(EventSubject* sender, EnvelopeKey* key)
:	ContentChangeEvent(sender)
,	m_key(key)
{
}

EnvelopeKey* EnvelopeContentChangeEvent::getKey() const
{
	return m_key;
}

	}
}

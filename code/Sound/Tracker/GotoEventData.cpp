/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Tracker/GotoEvent.h"
#include "Sound/Tracker/GotoEventData.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.GotoEventData", 0, GotoEventData, IEventData)

GotoEventData::GotoEventData(int32_t pattern, int32_t row)
:	m_pattern(pattern)
,	m_row(row)
{
}

Ref< IEvent > GotoEventData::createInstance() const
{
	return new GotoEvent(m_pattern, m_row);
}

void GotoEventData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"pattern", m_pattern);
	s >> Member< int32_t >(L"row", m_row);
}

}

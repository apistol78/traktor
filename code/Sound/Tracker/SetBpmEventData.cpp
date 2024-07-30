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
#include "Sound/Tracker/SetBpmEvent.h"
#include "Sound/Tracker/SetBpmEventData.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SetBpmEventData", 0, SetBpmEventData, IEventData)

SetBpmEventData::SetBpmEventData(int32_t bpm)
:	m_bpm(bpm)
{
}

Ref< IEvent > SetBpmEventData::createInstance() const
{
	return new SetBpmEvent(m_bpm);
}

void SetBpmEventData::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"bpm", m_bpm);
}

}

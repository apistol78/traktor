/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TheaterWorldComponentData.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/ActData.h"
#include "Theater/Performance.h"
#include "Theater/TheaterWorldComponent.h"

#include <algorithm>

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterWorldComponentData", 1, TheaterWorldComponentData, world::IWorldComponentData)

Ref< TheaterWorldComponent > TheaterWorldComponentData::createInstance(const world::IEntityBuilder* entityBuilder, bool editor) const
{
	// Do not randomize acts if in editor.
	Ref< Performance > performance = Performance::create(m_acts, !editor && m_randomizeActs, entityBuilder);
	if (!performance)
		return nullptr;

	return new TheaterWorldComponent(performance);
}

float TheaterWorldComponentData::getActStartTime(int32_t act) const
{
	float actStartTime = 0.0f;
	for (int32_t i = 0; i < std::min(act, int32_t(m_acts.size())); ++i)
		actStartTime += m_acts[i]->getDuration();
	return actStartTime;
}

void TheaterWorldComponentData::serialize(ISerializer& s)
{
	s >> MemberRefArray< ActData >(L"acts", m_acts);

	if (s.getVersion< TheaterWorldComponentData >() < 1)
	{
		// Repeating acts was never implemented; discarded in version 1.
		bool repeatActs = false;
		s >> Member< bool >(L"repeatActs", repeatActs);
	}

	s >> Member< bool >(L"randomizeActs", m_randomizeActs);
}

}

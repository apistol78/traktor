/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <random>
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/ActData.h"
#include "Theater/TheaterComponent.h"
#include "Theater/TheaterComponentData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterComponentData", 0, TheaterComponentData, world::IWorldComponentData)

Ref< TheaterComponent > TheaterComponentData::createInstance(bool editor) const
{
	float time = 0.0f;

	// Create act instances.
	RefArray< const Act > acts(m_acts.size());
	for (size_t i = 0; i < m_acts.size(); ++i)
	{
		const float duration = m_acts[i]->getDuration();
		if (duration < 0.0f)
			return nullptr;

		acts[i] = m_acts[i]->createInstance(time, time + duration);
		if (!acts[i])
			return nullptr;

		time += duration;
	}

	// Do not randomize acts if in editor.
	if (!editor && m_randomizeActs)
	{
		AlignedVector< size_t > indices(acts.size());
		for (size_t i = 0; i < indices.size(); ++i)
			indices[i] = i;

		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(indices.begin(), indices.end(), g);

		RefArray< const Act > tmp(m_acts.size());
		for (size_t i = 0; i < indices.size(); ++i)
			tmp[i] = acts[indices[i]];

		acts = tmp;
	}

	return new TheaterComponent(
		acts,
		time
	);
}

float TheaterComponentData::getActStartTime(int32_t act) const
{
	float actStartTime = 0.0f;
	for (int32_t i = 0; i < std::min(act, int32_t(m_acts.size())); ++i)
		actStartTime += m_acts[i]->getDuration();
	return actStartTime;
}

void TheaterComponentData::serialize(ISerializer& s)
{
	s >> MemberRefArray< ActData >(L"acts", m_acts, AttributePrivate());
	s >> Member< bool >(L"repeatActs", m_repeatActs);
	s >> Member< bool >(L"randomizeActs", m_randomizeActs);
}

}

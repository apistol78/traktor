/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <random>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Theater/ActData.h"
#include "Theater/TheaterController.h"
#include "Theater/TheaterControllerData.h"

namespace traktor
{
	namespace theater
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterControllerData", 1, TheaterControllerData, scene::ISceneControllerData)

TheaterControllerData::TheaterControllerData()
:	m_repeatActs(false)
,	m_randomizeActs(false)
{
}

Ref< scene::ISceneController > TheaterControllerData::createController(const SmallMap< Guid, Ref< world::Entity > >& entityProducts, bool editor) const
{
	float time = 0.0f;

	// Create act instances.
	RefArray< const Act > acts(m_acts.size());
	for (size_t i = 0; i < m_acts.size(); ++i)
	{
		float duration = m_acts[i]->getDuration();
		if (duration < 0.0f)
			return nullptr;

		acts[i] = m_acts[i]->createInstance(time, time + duration, entityProducts);
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

	return new TheaterController(
		acts,
		time
	);
}

float TheaterControllerData::getActStartTime(int32_t act) const
{
	float actStartTime = 0.0f;
	for (int32_t i = 0; i < std::min(act, int32_t(m_acts.size())); ++i)
		actStartTime += m_acts[i]->getDuration();
	return actStartTime;
}

void TheaterControllerData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< TheaterControllerData >() >= 1);

	s >> MemberRefArray< ActData >(L"acts", m_acts);
	s >> Member< bool >(L"repeatActs", m_repeatActs);
	s >> Member< bool >(L"randomizeActs", m_randomizeActs);
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/RefArray.h"
#include "World/IWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::theater
{

class Act;

/*! Theater world component.
 * \ingroup Theater
 */
class T_DLLCLASS TheaterComponent : public world::IWorldComponent
{
	T_RTTI_CLASS;

public:
	explicit TheaterComponent(const RefArray< const Act >& acts, float totalDuration);

	virtual void destroy() override final;

	virtual void update(world::World* world, const world::UpdateParams& update) override final;

	bool play(const std::wstring& actName);

private:
	friend class TheaterComponentEditor;

	RefArray< const Act > m_acts;
	double m_totalDuration = 0.0f;
	const Act* m_act = nullptr;
	double m_timeStart = -1.0;
	double m_timeLast = -1.0;
};

}

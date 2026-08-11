/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "World/IWorldComponent.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::theater
{

class Performance;

/*! Theater world component.
 *
 * Animate entities of the entire world.
 *
 * \ingroup Theater
 */
class T_DLLCLASS TheaterWorldComponent : public world::IWorldComponent
{
	T_RTTI_CLASS;

public:
	explicit TheaterWorldComponent(Performance* performance);

	virtual void destroy() override final;

	virtual void update(world::World* world, const world::UpdateParams& update) override final;

	bool play(const std::wstring& actName);

	void stop();

	bool isPlaying() const;

	Performance* getPerformance() const { return m_performance; }

private:
	Ref< Performance > m_performance;
};

}

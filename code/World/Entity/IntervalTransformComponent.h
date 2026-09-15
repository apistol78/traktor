/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/IntervalTransform.h"
#include "World/IWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Steps and interpolates registered interval transforms at guaranteed points.
 * \ingroup World
 */
class T_DLLCLASS IntervalTransformComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	struct Entry
	{
		Transform currentUpdate;
		Transform lastUpdate;
		Transform currentRender;
		Transform lastRender;
		uint32_t index;
	};

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	Entry* alloc();

	void free(Entry*& entry);

	void interpolate(float interval);

private:
	AlignedVector< Entry* > m_entries;
};

}

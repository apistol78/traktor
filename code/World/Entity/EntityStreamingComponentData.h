/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "World/IWorldComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class EntityData;

/*! Cells of entities streamed in and out of world by distance to a focus point.
 * \ingroup World
 */
class T_DLLCLASS EntityStreamingComponentData : public IWorldComponentData
{
	T_RTTI_CLASS;

public:
	struct Cell
	{
		Aabb3 boundingBox;
		RefArray< EntityData > entityData;

		void serialize(ISerializer& s);
	};

	void setLoadDistance(float loadDistance) { m_loadDistance = loadDistance; }

	float getLoadDistance() const { return m_loadDistance; }

	void setUnloadDistance(float unloadDistance) { m_unloadDistance = unloadDistance; }

	float getUnloadDistance() const { return m_unloadDistance; }

	void setCells(const AlignedVector< Cell >& cells) { m_cells = cells; }

	const AlignedVector< Cell >& getCells() const { return m_cells; }

	virtual void serialize(ISerializer& s) override final;

private:
	float m_loadDistance = 200.0f;
	float m_unloadDistance = 250.0f;
	AlignedVector< Cell > m_cells;
};

}

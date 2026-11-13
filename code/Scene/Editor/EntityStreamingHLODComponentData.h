/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::scene
{

/*! Editor-only entity streaming HLOD component.
 * \ingroup Scene
 *
 * Mark owner entity to be replaced by a stand-in, currently a billboard, in the HLOD
 * level of entity streaming; stripped when built.
 */
class T_DLLCLASS EntityStreamingHLODComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	int32_t getAngles() const { return m_angles; }

	int32_t getElevations() const { return m_elevations; }

	float getElevationFrom() const { return m_elevationFrom; }

	float getElevationTo() const { return m_elevationTo; }

	int32_t getTileSize() const { return m_tileSize; }

private:
	int32_t m_angles = 8;
	int32_t m_elevations = 1;
	float m_elevationFrom = 0.0f;
	float m_elevationTo = 0.0f;
	int32_t m_tileSize = 256;
};

}

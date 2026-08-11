/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Guid.h"
#include "Core/Containers/AlignedVector.h"
#include "Scene/Editor/ISceneOperationData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::terrain
{

/*! Operation data for the align-to-terrain scene operator.
 *
 * Describes a set of entities which should be vertically aligned to the terrain
 * surface during scene transformation.
 *
 * \ingroup Terrain
 */
class T_DLLCLASS AlignToTerrainOperationData : public scene::ISceneOperationData
{
	T_RTTI_CLASS;

public:
	/*! How the terrain below an entity's footprint is resolved into a single height. */
	enum class GroundFit
	{
		Center,	 //!< Sample terrain at the entity origin only; footprint is ignored.
		Lowest,	 //!< Lowest terrain below the footprint; no part of the entity is left floating.
		Average, //!< Mean terrain height below the footprint.
		Highest	 //!< Highest terrain below the footprint; no part of the entity penetrates the terrain.
	};

	/*! Name of the layers whose entities are aligned. */
	const AlignedVector< std::wstring >& getLayers() const { return m_layers; }

	/*! Whether entity orientation is aligned to the terrain surface normal. */
	bool getAlignOrientation() const { return m_alignOrientation; }

	/*! Apply random local heading angle to entity. */
	bool getRandomHeadingAngle() const { return m_randomHeadingAngle; }

	/*! Blend of the surface normal toward straight up; 0 = follow normal, 1 = upright. */
	float getUpness() const { return m_upness; }

	/*! Vertical offset added after snapping to the terrain surface. */
	float getOffset() const { return m_offset; }

	/*! How the terrain below an entity's projected bounding box is resolved into a single height. */
	GroundFit getGroundFit() const { return m_groundFit; }

	/*! Fraction of an entity's height, measured up from its lowest point, which is
	 * considered to be in contact with the ground.
	 *
	 * Only geometry within this slab defines the footprint projected onto the
	 * terrain, so a tree is measured by its trunk rather than by its canopy.
	 * A ratio of 1 uses the entity's entire bounding box.
	 */
	float getContactRatio() const { return m_contactRatio; }

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< std::wstring > m_layers;
	bool m_alignOrientation = false;
	bool m_randomHeadingAngle = false;
	float m_upness = 1.0f;
	float m_offset = 0.0f;
	GroundFit m_groundFit = GroundFit::Lowest;
	float m_contactRatio = 0.1f;
};

}

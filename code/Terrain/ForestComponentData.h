/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Const.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Id.h"
#include "Terrain/TerrainLayerComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class InstanceMesh;

}

namespace traktor::terrain
{

class T_DLLCLASS ForestComponentData : public TerrainLayerComponentData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class ForestComponent;
	friend class TerrainEntityPipeline;

	resource::Id< mesh::InstanceMesh > m_lod0mesh;
	resource::Id< mesh::InstanceMesh > m_lod1mesh;
	resource::Id< mesh::InstanceMesh > m_lod2mesh;

	uint8_t m_attribute = 0;
	float m_density = 0.1f;
	float m_lod0distance = 50.0f;
	float m_lod1distance = 100.0f;
	float m_lod2distance = 200.0f;
	float m_upness = 0.8f;		//!< Upness of tree, 0 means follow normal of terrain and 1 means completely straight up.
	float m_randomScale = 0.2f;
	float m_randomTilt = deg2rad(4.0f);
	float m_slopeAngleThreshold = deg2rad(45.0f);
};

}

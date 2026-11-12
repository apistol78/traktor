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
#include "Core/Guid.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ai
{

class NavMeshAsset;

/*! Single chunk of a chunked navigation mesh; synthesized by navigation mesh pipeline.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshChunkAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Terrain
	{
		Guid terrain;
		Transform transform;

		void serialize(ISerializer& s);
	};

	struct Modifier
	{
		Guid id;
		Aabb3 bounds;
		int32_t area = 0;

		void serialize(ISerializer& s);
	};

	Ref< NavMeshAsset > m_navMesh;			//!< Settings and source of navigation mesh.
	int32_t m_chunkX = 0;
	int32_t m_chunkZ = 0;
	int32_t m_chunkTiles = 0;				//!< Number of tiles along each side of chunk.
	Vector4 m_origin = Vector4::zero();		//!< Origin of first chunk in grid.
	float m_oceanHeight = 0.0f;
	bool m_oceanClip = false;
	AlignedVector< Guid > m_entities;		//!< Entities with geometry overlapping chunk.
	AlignedVector< Terrain > m_terrains;
	AlignedVector< Modifier > m_modifiers;
	uint32_t m_stamp = 0;					//!< Stamp of entities and terrain tiles read; output depend on stamp instead of source scene.

	virtual void serialize(ISerializer& s) override final;
};

}

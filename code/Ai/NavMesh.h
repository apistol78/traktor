/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Guid.h"
#include "Core/Math/Aabb3.h"
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ai
{

class NavMeshChunk;

/*! Navigation mesh.
 * \ingroup AI
 *
 * Immutable description of a navigation mesh, which is
 * instantiated and queried through a NavMeshComponent.
 */
class T_DLLCLASS NavMesh : public Object
{
	T_RTTI_CLASS;

private:
	friend class NavMeshComponent;
	friend class NavMeshComponentEditor;
	friend class NavMeshFactory;

	struct Modifier
	{
		Aabb3 bounds;
		uint8_t area = 0;
	};

	struct Chunk
	{
		int32_t x = 0;
		int32_t z = 0;
		Guid id;
	};

	float m_origin[3] = { 0.0f, 0.0f, 0.0f };
	float m_tileWidth = 0.0f;
	float m_tileHeight = 0.0f;
	int32_t m_maxTiles = 0;
	int32_t m_maxPolys = 0;
	SmallMap< Guid, Modifier > m_modifiers;
	Ref< NavMeshChunk > m_tiles;	//!< Tiles of a navigation mesh which isn't chunked.
	AlignedVector< Chunk > m_chunks;	//!< Chunks of a chunked navigation mesh.
	float m_chunkWidth = 0.0f;
};

}

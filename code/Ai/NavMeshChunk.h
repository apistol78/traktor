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
#include "Core/Math/Aabb3.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ai
{

/*! Tiles of a navigation mesh, or of a chunk of a chunked navigation mesh.
 * \ingroup AI
 */
class T_DLLCLASS NavMeshChunk : public Object
{
	T_RTTI_CLASS;

private:
	friend class NavMeshComponent;
	friend class NavMeshFactory;

	AlignedVector< AlignedVector< uint8_t > > m_tiles;	//!< Detour tile data, copied when added to navigation mesh.
	Aabb3 m_bounds;	//!< Bounds of tiles.
};

}

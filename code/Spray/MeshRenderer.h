/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Spray/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class InstanceMesh;

}

namespace traktor::render
{

class RenderContext;

}

namespace traktor::world
{

class IWorldRenderPass;

}

namespace traktor::spray
{

/*! Mesh particle renderer.
 * \ingroup Spray
 */
class T_DLLCLASS MeshRenderer : public Object
{
	T_RTTI_CLASS;

public:
	void render(
		mesh::InstanceMesh* mesh,
		bool meshOrientationFromVelocity,
		const pointVector_t& points
	);

	void flush(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	SmallMap< Ref< mesh::InstanceMesh >, std::pair< pointVector_t, bool > > m_meshes;
	//AlignedVector< mesh::InstanceMesh::RenderInstance > m_instances;
};

}

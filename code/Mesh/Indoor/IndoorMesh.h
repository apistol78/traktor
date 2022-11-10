/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Winding3.h"
#include "Mesh/IMesh.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class RenderContext;
class Mesh;
class ITexture;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldRenderView;

}

namespace traktor::mesh
{

class IMeshParameterCallback;

class T_DLLCLASS IndoorMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	void build(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		const Transform& worldTransform,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

private:
	friend class IndoorMeshResource;

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
	};

	struct Sector
	{
		Aabb3 boundingBox;
		SmallMap< render::handle_t, std::vector< Part > > parts;
	};

	struct Portal
	{
		Winding3 winding;
		int sectorA;
		int sectorB;
	};

	Ref< render::Mesh > m_mesh;
	resource::Proxy< render::Shader > m_shader;
	std::vector< Part > m_parts;
	AlignedVector< Sector > m_sectors;
	AlignedVector< Portal > m_portals;

	void findVisibleSectors(
		const AlignedVector< Plane >& frustum,
		const Matrix44& view,
		int currentSector,
		SmallSet< int >& outVisibleSectors
	);
};

}

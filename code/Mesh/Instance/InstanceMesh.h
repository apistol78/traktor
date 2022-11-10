/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector4.h"
#include "Mesh/IMesh.h"
#include "Mesh/Instance/InstanceMeshData.h"
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

class ProgramParameters;
class RenderContext;
class Mesh;
class Shader;

}

namespace traktor::world
{

class IWorldCulling;
class IWorldRenderPass;

}

namespace traktor::mesh
{

/*! Instance mesh.
 *
 * Instance meshes are meshes which are repeated
 * automatically by the GPU in any number of instances
 * using hardware instancing in a single draw call.
 */
class T_DLLCLASS InstanceMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	enum { MaxInstanceCount = 60 };

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
	};

	struct RenderInstance
	{
		InstanceMeshData data;
		InstanceMeshData data0;
		float distance;

		RenderInstance() = default;

		RenderInstance(const InstanceMeshData& data_, const InstanceMeshData& data0_, float distance_)
		:	data(data_)
		,	data0(data0_)
		,	distance(distance_)
		{
		}
	};

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	void getTechniques(SmallSet< render::handle_t >& outHandles) const;

	void build(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		AlignedVector< RenderInstance >& instanceWorld,
		render::ProgramParameters* extraParameters
	) const;

private:
	friend class InstanceMeshResource;

	resource::Proxy< render::Shader > m_shader;
	Ref< render::Mesh > m_renderMesh;
	SmallMap< render::handle_t, AlignedVector< Part > > m_parts;
	int32_t m_maxInstanceCount = 0;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Transform.h"
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

class Buffer;
class IAccelerationStructure;
class ITexture;
class Mesh;
class RenderContext;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldRenderView;
class WorldSetupContext;

}

namespace traktor::mesh
{

class IMeshParameterCallback;

/*! Static mesh.
 * \ingroup Mesh
 *
 * A static mesh is a basic rigid mesh which at
 * all times should be placed on the GPU for
 * quick rendering.
 */
class T_DLLCLASS StaticMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
	};

	typedef std::pair< uint32_t, uint32_t > techniqueParts_t;

	const Aabb3& getBoundingBox() const;

	const techniqueParts_t* findTechniqueParts(render::handle_t technique) const;

	void build(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		const techniqueParts_t& techniqueParts,
		const Transform& lastWorldTransform,
		const Transform& worldTransform,
		float distance,
		const IMeshParameterCallback* parameterCallback
	);

	const render::IAccelerationStructure* getAccelerationStructure() const { return m_rtAccelerationStructure; }

	const render::Buffer* getRTVertexAttributes() const;

private:
	friend class StaticMeshResource;

	AlignedVector< Part > m_parts;
	SmallMap< render::handle_t, techniqueParts_t > m_techniqueParts;

	// Rasterization
	resource::Proxy< render::Shader > m_shader;
	Ref< render::Mesh > m_renderMesh;
	
	// Ray tracing
	Ref< render::IAccelerationStructure > m_rtAccelerationStructure;

#if defined(_DEBUG)
	std::string m_name;
#endif
};

}

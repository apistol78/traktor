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
class IRenderSystem;
class ITexture;
class IVertexLayout;
class RenderContext;
class Mesh;

}

namespace traktor::world
{

class IWorldRenderPass;

}

namespace traktor::mesh
{

class IMeshParameterCallback;

/*! Skinned mesh.
 * \ingroup Mesh
 *
 * For each vertex the skinned mesh blends
 * the final world transform from a palette of
 * transforms using per-vertex weights.
 */
class T_DLLCLASS SkinnedMesh : public IMesh
{
	T_RTTI_CLASS;

public:
#pragma pack(1)
	struct JointData
	{
		float translation[4];
		float rotation[4];
	};
#pragma pack()

	static const FourCC c_fccSkinPosition;

	SkinnedMesh() = default;

	const Aabb3& getBoundingBox() const;

	bool supportTechnique(render::handle_t technique) const;

	void buildSkin(
		render::RenderContext* renderContext,
		render::Buffer* jointTransforms,
		render::Buffer* skinBuffer
	) const;

	void buildAccelerationStructure(
		render::RenderContext* renderContext,
		render::Buffer* skinBuffer,
		render::IAccelerationStructure* accelerationStructure
	) const;

	void build(
		render::RenderContext* renderContext,
		const world::IWorldRenderPass& worldRenderPass,
		const Transform& lastWorldTransform,
		const Transform& worldTransform,
		render::Buffer* lastSkinBuffer,
		render::Buffer* skinBuffer,
		float distance,
		const IMeshParameterCallback* parameterCallback
	) const;

	int32_t getJointCount() const;

	const SmallMap< std::wstring, int32_t >& getJointMap() const;

	Ref< render::Buffer > createSkinBuffer(render::IRenderSystem* renderSystem) const;

	static Ref< render::Buffer > createJointBuffer(render::IRenderSystem* renderSystem, uint32_t jointCount);

	Ref< render::IAccelerationStructure > createAccelerationStructure(render::IRenderSystem* renderSystem) const;

	const render::Buffer* getRTTriangleAttributes() const;

	const render::Buffer* getRTVertexAttributes() const;

	const render::Mesh* getRenderMesh() const { return m_mesh; }

private:
	friend class SkinnedMeshResource;

	struct Part
	{
		render::handle_t shaderTechnique;
		uint32_t meshPart;
	};
	
	SmallMap< render::handle_t, AlignedVector< Part > > m_parts;
	SmallMap< std::wstring, int32_t > m_jointMap;
	int32_t m_jointCount = 0;

	// Rasterization
	resource::Proxy< render::Shader > m_shaderUpdateSkin;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::Mesh > m_mesh;

	// Ray tracing.
	Ref< const render::IVertexLayout > m_rtVertexLayout;

#if defined(_DEBUG)
	std::string m_name;
#endif
};

}

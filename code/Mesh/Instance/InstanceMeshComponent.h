/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Mesh/MeshComponent.h"
#include "Resource/Proxy.h"
#include "World/Entity/CullingComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class InstanceMesh;

/*! Instancing mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS InstanceMeshComponent
:	public MeshComponent
,	public world::CullingComponent::ICullable
{
	T_RTTI_CLASS;

public:
	explicit InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh);

	virtual ~InstanceMeshComponent();

	virtual void destroy() override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

	inline resource::Proxy< InstanceMesh >& getMesh() { return m_mesh; }

	/* world::CullingComponent::ICullable */

	virtual Aabb3 cullableGetBoundingBox() const override final { return getBoundingBox(); }

	virtual void cullableBuild(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		render::Buffer* instanceBuffer,
		render::Buffer* visibilityBuffer,
		uint32_t start,
		uint32_t count
	) override final;

private:
	resource::Proxy< InstanceMesh > m_mesh;
	world::World* m_world = nullptr;
	world::CullingComponent::Instance* m_cullingInstance = nullptr;
};

}

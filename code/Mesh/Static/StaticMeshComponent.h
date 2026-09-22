/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Mesh/MeshComponent.h"
#include "Resource/Proxy.h"
#include "World/Entity/RTWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::mesh
{

class StaticMesh;

/*! Static mesh component.
 * \ingroup Mesh
 *
 * Meshes with materials providing a world position offset are deformed on the
 * compute queue while within deform distance; the component then holds a slot
 * of its mesh's deform pool and, when ray tracing nearby, the slot's refit
 * acceleration structure replaces the shared one.
 */
class T_DLLCLASS StaticMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit StaticMeshComponent(const resource::Proxy< StaticMesh >& mesh, render::IRenderSystem* renderSystem);

	virtual void destroy() override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setState(const world::EntityState& state, const world::EntityState& mask, bool includeChildren) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	void setup(const world::WorldSetupContext& context, const world::WorldRenderView& worldRenderView);

	/*! Acquire or release the deform slot depending on distance to the eye; deformed meshes only.
	 *
	 * \return True if the component holds a deform slot and must be deformed this frame.
	 */
	bool setupDeform(const world::WorldRenderView& worldRenderView);

	/*! Build compute work deforming this instance into its slot. */
	void buildDeform(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext);

	/*! Build update of the slot's acceleration structure and point the ray tracing instance at it. */
	void buildDeformAccelerationStructure(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext);

	void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass);

	bool haveDeformSlot() const { return m_deformSlot >= 0; }

	/*! Check if this component has a ray tracing instance which may follow the deformation. */
	bool haveAccelerationStructure() const { return m_rtwInstance != nullptr; }

	const Transform& getRenderTransform() const { return m_transform->currentRender; }

	resource::Proxy< StaticMesh >& getMesh() { return m_mesh; }

private:
	resource::Proxy< StaticMesh > m_mesh;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;
	int32_t m_deformSlot = -1;
	StaticMesh* m_deformSlotMesh = nullptr; //!< Mesh the slot was allocated from; a reloaded mesh knows nothing of it.
	bool m_deformSlotNew = false;			//!< Slot acquired this frame; its history must be written too.

	void releaseDeformSlot();
};

}

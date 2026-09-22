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
#include "World/Entity/CullingComponent.h"
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

class RenderContext;

}

namespace traktor::mesh
{

class InstanceMesh;

/*! Instancing mesh component.
 * \ingroup Mesh
 *
 * Deformed instance meshes hold a deform slot of their mesh while within deform
 * distance; the slot is written every frame and carried by the culling instance
 * so the instanced draw picks the deformed positions for this instance.
 */
class T_DLLCLASS InstanceMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh);

	virtual ~InstanceMeshComponent();

	virtual void destroy() override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setState(const world::EntityState& state, const world::EntityState& mask, bool includeChildren) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	void setup(
		const world::WorldSetupContext& context,
		const world::WorldRenderView& worldRenderView
	);

	/*! Acquire or release the deform slot depending on distance to the eye; deformed meshes only.
	 *
	 * \return True if the instance holds a deform slot and must be deformed this frame.
	 */
	bool setupDeform(const world::WorldRenderView& worldRenderView);

	/*! Build compute work deforming this instance into its slot. */
	void buildDeform(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext);

	/*! Build update of the slot's acceleration structure and point the ray tracing instance at it. */
	void buildDeformAccelerationStructure(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext);

	bool haveDeformSlot() const { return m_deformSlot >= 0; }

	int32_t getDeformSlot() const { return m_deformSlot; }

	/*! Index of this instance within its culling batch; valid while culled. */
	bool getBatchIndex(uint32_t& outBatchIndex) const;

	inline resource::Proxy< InstanceMesh >& getMesh() { return m_mesh; }

private:
	resource::Proxy< InstanceMesh > m_mesh;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;
	world::CullingComponent::Instance* m_cullingInstance = nullptr;
	int32_t m_deformSlot = -1;
	InstanceMesh* m_deformSlotMesh = nullptr; //!< Mesh the slot was allocated from; a reloaded mesh knows nothing of it.
	bool m_deformSlotNew = false;			  //!< Slot acquired this frame; its history must be written too.
	bool m_dynamic = false;

	void releaseDeformSlot();
};

}

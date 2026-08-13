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
#include "Core/Math/Matrix44.h"
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

class Buffer;
class IAccelerationStructure;
class IRenderSystem;

}

namespace traktor::mesh
{

class SkinnedMesh;

/*! Skinned mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS SkinnedMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit SkinnedMeshComponent(const resource::Proxy< SkinnedMesh >& mesh, render::IRenderSystem* renderSystem);

	virtual void destroy() override;

	virtual void setWorld(world::World* world) override;

	virtual void setState(const world::EntityState& state, const world::EntityState& mask, bool includeChildren) override;

	virtual void setTransform(const Transform& transform) override;

	virtual Aabb3 getBoundingBox() const override;

	virtual void setupSkin(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext, int32_t lodRank);

	virtual void setupAccelerationStructure(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext, int32_t lodRank);

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass);

	void setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms);

	/*! Check if this component maintains a ray tracing acceleration structure instance. */
	bool haveAccelerationStructure() const { return m_rtwInstance != nullptr; }

protected:
	/*! Number of skin buffer ring slots.
	 *
	 * The skin is written on the asynchronous compute queue while up to the swap
	 * chain's image count (at most 4) of prior frames' graphics may still be
	 * reading their slots; the previous frame's slot is additionally bound for
	 * velocities, so a slot is read for up to five frames after being written.
	 * Writing a slot an in-flight frame still reads shows up as z-fighting
	 * between the z pre-pass and the g-buffer pass, or as skin corruption.
	 */
	constexpr static int32_t SkinBufferCount = 6;

	resource::Proxy< SkinnedMesh > m_mesh;
	world::World* m_world = nullptr;
	Ref< render::Buffer > m_jointBuffer;
	Ref< render::Buffer > m_skinBuffer[SkinBufferCount];
	Ref< render::IAccelerationStructure > m_rtAccelerationStructure;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;
	int32_t m_rtUpdates = 0;
	bool m_setupBuiltSkin = false;
};

}

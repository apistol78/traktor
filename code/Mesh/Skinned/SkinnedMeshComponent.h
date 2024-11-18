/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

	virtual void destroy() override final;

	virtual void setWorld(world::World* world) override final;

	virtual void setState(const world::EntityState& state, const world::EntityState& mask) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	void setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms);

private:
	resource::Proxy< SkinnedMesh > m_mesh;
	world::World* m_world = nullptr;

	Ref< render::Buffer > m_jointBuffer;
	Ref< render::Buffer > m_skinBuffer[2];

	Ref< render::IAccelerationStructure > m_rtAccelerationStructure;
	world::RTWorldComponent::Instance* m_rtwInstance = nullptr;
};

}

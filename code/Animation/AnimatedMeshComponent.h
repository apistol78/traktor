/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <atomic>
#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Job.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "Mesh/MeshComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class SkinnedMesh;

}

namespace traktor::render
{

class Buffer;
class IRenderSystem;

}

namespace traktor::animation
{

class Skeleton;

/*! Animated mesh entity.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshComponent : public mesh::MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit AnimatedMeshComponent(
		const Transform& transform,
		const resource::Proxy< mesh::SkinnedMesh >& mesh,
		render::IRenderSystem* renderSystem
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	/*! Get skin transform of joint in delta space. */
	bool getSkinTransform(render::handle_t jointName, Transform& outTransform) const;

private:
	resource::Proxy< mesh::SkinnedMesh > m_mesh;
	AlignedVector< int32_t > m_jointRemap;
	AlignedVector< Transform > m_jointInverseTransforms;
	AlignedVector< Transform > m_poseTransforms[2];
	Ref< render::Buffer > m_jointBuffers[2];
	Transform m_lastWorldTransform[2];
	std::atomic< int32_t > m_index;
};

}

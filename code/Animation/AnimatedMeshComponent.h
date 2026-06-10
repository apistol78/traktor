/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"

#include <atomic>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class Skeleton;

/*! Animated mesh entity.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshComponent : public mesh::SkinnedMeshComponent
{
	T_RTTI_CLASS;

public:
	explicit AnimatedMeshComponent(
		const Transform& transform,
		const resource::Proxy< mesh::SkinnedMesh >& mesh,
		render::IRenderSystem* renderSystem);

	virtual void setOwner(world::Entity* owner) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	virtual bool setup(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext) override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	/*! Get skin transform of joint in delta space. */
	bool getSkinTransform(render::handle_t jointName, Transform& outTransform) const;

private:
	AlignedVector< int32_t > m_jointRemap;
	AlignedVector< Transform > m_jointInverseTransforms;
	AlignedVector< Transform > m_poseTransforms[2];
	Transform m_lastWorldTransform[2];
	std::atomic< int32_t > m_index;
	int32_t m_revision = -1;
	bool m_skinModified = false;
	bool m_lastIsVisible = false;
};

}

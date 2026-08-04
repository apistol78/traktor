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
#include <limits>

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
class SkeletonComponent;

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

	virtual bool setupSkin(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext, int32_t lodRank) override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	/*! Get skin transform of joint in delta space. */
	bool getSkinTransform(render::handle_t jointName, Transform& outTransform) const;

	/*! Get closest culling distance across all views. */
	float getLastDistance() const { return m_lastDistance; }

	/*! Get number of updates between pose evaluations. \sa SkeletonComponent::setUpdatePeriod */
	int32_t getUpdatePeriod() const { return m_updatePeriod; }

private:
	SkeletonComponent* m_skeletonComponent = nullptr;
	AlignedVector< int32_t > m_jointRemap;
	AlignedVector< Transform > m_jointInverseTransforms;
	AlignedVector< Transform > m_poseTransforms[2];	//!< Pose of two last updates; m_index refer to the most recent.
	Transform m_lastWorldTransform[2];
	std::atomic< int32_t > m_index;
	int32_t m_revision = -1;
	bool m_skinModified = false;
	bool m_skinBuilt = false;	//!< Skin has been rebuilt this frame, thus previous skin buffer is valid.
	bool m_firstSetup = true;
	bool m_visibleThisFrame = false;	//!< Visible in any pass, of any view, this frame.
	bool m_visibleLastFrame = true;
	int32_t m_updatePeriod = 1;
	float m_lastDistance = std::numeric_limits< float >::max();
};

}

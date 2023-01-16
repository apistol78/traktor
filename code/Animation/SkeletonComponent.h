/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Job.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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
class IPoseController;

/*! Skeleton entity component.
 * \ingroup Animation
 */
class T_DLLCLASS SkeletonComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit SkeletonComponent(
		const Transform& transform,
		const resource::Proxy< Skeleton >& skeleton,
		IPoseController* poseController
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void synchronize() const;

	/*! Get base transform of joint. */
	bool getJointTransform(render::handle_t jointName, Transform& outTransform) const;

	/*! Get transform of current pose. */
	bool getPoseTransform(render::handle_t jointName, Transform& outTransform) const;

	/*! Set pose transform of joint. */
	bool setPoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive);

	/*! Get skeleton. */
	const resource::Proxy< Skeleton >& getSkeleton() const { return m_skeleton; }

	/*! Set pose evaluation controller. */
	void setPoseController(IPoseController* poseController) { m_poseController = poseController; }

	/*! Get pose evaluation controller. */
	IPoseController* getPoseController() const { return m_poseController; }

	/*! Get all joint base transforms. */
	const AlignedVector< Transform >& getJointTransforms() const { return m_jointTransforms; }

	/*! Get all joint pose transforms. */
	const AlignedVector< Transform >& getPoseTransforms() const { return m_poseTransforms; }

private:
	Transform m_transform;
	resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	AlignedVector< Transform > m_jointTransforms;
	AlignedVector< Transform > m_poseTransforms;
	mutable Ref< Job > m_updatePoseControllerJob;

	void updatePoseController(float time, float deltaTime);
};

}

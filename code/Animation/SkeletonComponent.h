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
#include "Core/RefArray.h"
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
	/*! Largest supported pose evaluation period.
	 *
	 * Also the wrap of the interleave counter, thus every period used should be a
	 * divisor of this value in order to keep the evaluation rate even across the wrap.
	 * \sa setUpdatePeriod
	 */
	const static int32_t c_maxUpdatePeriod = 16;

	explicit SkeletonComponent(
		const Transform& transform,
		const resource::Proxy< Skeleton >& skeleton,
		IPoseController* poseController);

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

	/*! Concatenate pose transform of joint. */
	bool concatenatePoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive);

	/*! Get skeleton. */
	const resource::Proxy< Skeleton >& getSkeleton() const { return m_skeleton; }

	/*! Set pose evaluation controller. */
	void setPoseController(IPoseController* poseController) { m_poseController = poseController; }

	/*! Get pose evaluation controller. */
	IPoseController* getPoseController() const { return m_poseController; }

	/*! The pose controller currently driving the pose (e.g. the rag doll controller while
	 * in a rag doll state), or null. See IPoseController::getActivePoseController. */
	IPoseController* getActivePoseController() const;

	/*! */
	void getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const;

	/*! Get all joint base transforms. */
	const AlignedVector< Transform >& getJointTransforms() const { return m_jointTransforms; }

	/*! Get all joint pose transforms. */
	const AlignedVector< Transform >& getPoseTransforms() const { return m_poseTransforms; }

	/*! Set all joint pose transforms. */
	void setPoseTransforms(const AlignedVector< Transform >& poseTransforms)
	{
		m_poseTransforms = poseTransforms;
		m_revision++;
	}

	/*! Get update revision. */
	int32_t getRevision() const { return m_revision; }

	/*! Set pose evaluation period.
	 *
	 * Number of updates between each pose controller evaluation; 1, the default, evaluate
	 * the pose every update. Used to evaluate distant skeletons at a reduced rate. Time of
	 * skipped updates is accumulated so the animation still progress at the correct rate.
	 * Clamped to [1, c_maxUpdatePeriod].
	 */
	void setUpdatePeriod(int32_t updatePeriod)
	{
		m_updatePeriod = (updatePeriod < 1) ? 1 : ((updatePeriod > c_maxUpdatePeriod) ? c_maxUpdatePeriod : updatePeriod);
	}

	/*! Get pose evaluation period. */
	int32_t getUpdatePeriod() const { return m_updatePeriod; }

private:
	world::Entity* m_owner = nullptr;
	Transform m_transform;
	resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	AlignedVector< Transform > m_jointTransforms;
	AlignedVector< Transform > m_poseTransforms;
	mutable Ref< Job > m_updatePoseControllerJob;
	std::atomic< int32_t > m_revision;
	mutable Aabb3 m_boundingBox;
	mutable int32_t m_boundingBoxRevision = -1;
	std::atomic< int32_t > m_updatePeriod = 1;
	int32_t m_updatePhase = 0;
	int32_t m_updateCount = 0;
	double m_deferredDeltaTime = 0.0;

	void updatePoseController(double time, double deltaTime);

	void applyEntityTransform();
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/IPoseControllerData.h"
#include "Resource/Id.h"

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

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RetargetPoseControllerData : public IPoseControllerData
{
	T_RTTI_CLASS;

public:
	RetargetPoseControllerData() = default;

	explicit RetargetPoseControllerData(const resource::Id< Skeleton >& animationSkeleton);

	virtual Ref< IPoseController > createInstance(
		resource::IResourceManager* resourceManager,
		physics::PhysicsManager* physicsManager,
		const Skeleton* skeleton,
		const Transform& worldTransform
	) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Skeleton >& getAnimationSkeleton() const { return m_animationSkeleton; }

	const IPoseControllerData* getPoseController() const { return m_poseController; }

private:
	resource::Id< Skeleton > m_animationSkeleton;
	Ref< const IPoseControllerData > m_poseController;
};

}

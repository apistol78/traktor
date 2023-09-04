/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/IPoseController.h"
#include "Resource/Proxy.h"

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
class T_DLLCLASS RetargetPoseController : public IPoseController
{
	T_RTTI_CLASS;

public:
	explicit RetargetPoseController(const resource::Proxy< Skeleton >& animationSkeleton, IPoseController* poseController);

	virtual void destroy() override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms
	) override final;

	IPoseController* getController() const { return m_poseController; }

private:
	resource::Proxy< Skeleton > m_animationSkeleton;
	Ref< IPoseController > m_poseController;
	AlignedVector< Transform > m_jointTransforms;
	AlignedVector< Transform > m_poseTransforms;
	AlignedVector< Transform > m_remappedPoseTransforms;
	AlignedVector< Transform > m_deltaPoseTransforms;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/StateGraph/RtStateNode.h"
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

class Animation;
class ITransformTime;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateNodeAnimation : public RtStateNode
{
	T_RTTI_CLASS;

public:
	virtual void destroy() override final;

	virtual void reset(
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& poseTransforms) override final;

	virtual float evaluate(
		const RtStateContext& context,
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		Pose& outPose,
		Vector4* outRootMotion) override final;

	virtual IPoseController* getActivePoseController() const override final;

	virtual void getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const override final;

private:
	friend class RtStateNodeAnimationData;

	resource::Proxy< Animation > m_animation;
	Ref< ITransformTime > m_transformTime;

	float m_duration = 0.0f;
	float m_time = 0.0f;
	float m_lastTime = 0.0f;
};

}

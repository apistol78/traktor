/*
 * TRAKTOR
 * Copyright (c) 2025-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Pose.h"
#include "Animation/StateGraph/RtStateContext.h"
#include "Core/Math/Transform.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class IPoseController;
class RtStateNode;
class RtStateTransition;
class Skeleton;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateGraph : public Object
{
	T_RTTI_CLASS;

public:
	void destroy();

	bool setParameterValue(const render::Handle& handle, bool value);

	bool getParameterValue(const render::Handle& handle) const;

	bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms);

	Vector4 consumeRootMotion();

	IPoseController* getActivePoseController() const;

	const RtStateNode* getRootState() const { return m_rootState; }

	const RtStateNode* getCurrentState() const { return m_currentState; }

	/*! State being blended into, null if none. */
	const RtStateNode* getNextState() const { return m_nextState; }

	void getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const;

	const RefArray< RtStateNode >& getStates() const { return m_states; }

private:
	friend class RtStateGraphData;
	friend class RtStateTransitionData;

	RefArray< RtStateNode > m_states;
	RefArray< RtStateTransition > m_transitions;
	RtStateNode* m_rootState = nullptr;
	RtStateNode* m_currentState = nullptr;
	RtStateNode* m_nextState = nullptr;
	RtStateContext m_context;
	Pose m_evaluatePose;
	float m_blendState = 0.0f;
	float m_blendDuration = 0.0;
	float m_timeFactor = 1.0f;
	Vector4 m_rootMotion = Vector4::zero();

	/*! Switch to state at once, from the pose shown. */
	void cut(RtStateNode* state, const Transform& worldTransform, const Skeleton* skeleton, const AlignedVector< Transform >& poseTransforms);
};

}

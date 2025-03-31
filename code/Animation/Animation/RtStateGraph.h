/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Animation/StateContext.h"
#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"
#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

class RtState;
class RtStateTransition;
class Skeleton;

/*!
 * \ingroup Animation
 */
class T_DLLCLASS RtStateGraph : public Object
{
	T_RTTI_CLASS;

public:
	bool evaluate(
		float time,
		float deltaTime,
		const Transform& worldTransform,
		const Skeleton* skeleton,
		const AlignedVector< Transform >& jointTransforms,
		AlignedVector< Transform >& outPoseTransforms);

private:
	friend class RtStateGraphData;

	RefArray< RtState > m_states;
	RefArray< RtStateTransition > m_transitions;
	RtState* m_rootState = nullptr;
	RtState* m_currentState = nullptr;
	RtState* m_nextState = nullptr;
	StateContext m_currentStateContext;
	StateContext m_nextStateContext;
	Pose m_evaluatePose;
	float m_blendState = 0.0f;
	float m_blendDuration = 0.0;
	float m_timeFactor = 1.0f;
};

}

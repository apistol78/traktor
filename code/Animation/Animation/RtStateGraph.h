/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Transform.h"

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
		AlignedVector< Transform >& outPoseTransforms
	);

private:
	RefArray< RtState > m_states;
    RefArray< RtStateTransition > m_transitions;
	RtState* m_rootState = nullptr;
	RtState* m_currentState = nullptr;
	RtState* m_nextState = nullptr;
	float m_blendState = 0.0f;
	float m_blendDuration = 0.0;
};

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/SkeletonUtils.h"
#include "Animation/Animation/StateNode.h"
#include "Animation/Animation/StateNodeController.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateNodeController", StateNodeController, IPoseController)

StateNodeController::StateNodeController(StateNode* node)
:	m_node(node)
,	m_initialized(false)
{
}

void StateNodeController::destroy()
{
	m_node = nullptr;
}

void StateNodeController::setTransform(const Transform& transform)
{
}

bool StateNodeController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	Pose currentPose;

	if (!m_initialized)
	{
		if (!m_node || !m_node->prepareContext(m_context))
			return false;
		m_initialized = true;
	}

	m_node->evaluate(
		m_context,
		currentPose
	);

	m_context.setTime(time);

	float timeLeft = m_context.getDuration() - m_context.getTime();
	if (timeLeft <= 0.0f)
		m_initialized = false;

	calculatePoseTransforms(
		skeleton,
		&currentPose,
		outPoseTransforms
	);

	return true;
}

void StateNodeController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/AnimationGraphPoseController.h"

#include "Animation/StateGraph/RtStateGraph.h"
#include "Animation/StateGraph/RtStateNode.h"
#include "Core/Misc/SafeDestroy.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationGraphPoseController", AnimationGraphPoseController, IPoseController)

AnimationGraphPoseController::AnimationGraphPoseController(RtStateGraph* stateGraph)
	: m_stateGraph(stateGraph)
{
}

bool AnimationGraphPoseController::setParameterValue(const render::Handle& parameter, bool value)
{
	if (m_stateGraph)
		return m_stateGraph->setParameterValue(parameter, value);
	else
		return false;
}

const RtStateNode* AnimationGraphPoseController::getRootState() const
{
	return m_stateGraph ? m_stateGraph->getRootState() : nullptr;
}

const RtStateNode* AnimationGraphPoseController::getCurrentState() const
{
	return m_stateGraph ? m_stateGraph->getCurrentState() : nullptr;
}

const RtStateNode* AnimationGraphPoseController::getNextState() const
{
	return m_stateGraph ? m_stateGraph->getNextState() : nullptr;
}

void AnimationGraphPoseController::destroy()
{
	safeDestroy(m_stateGraph);
}

void AnimationGraphPoseController::setOwner(world::Entity* owner)
{
	for (auto state : m_stateGraph->getStates())
	{
		RefArray< IPoseController > poseControllers;
		state->getPoseControllersOf(type_of< IPoseController >(), poseControllers);
		for (auto poseController : poseControllers)
			poseController->setOwner(owner);
	}
}

void AnimationGraphPoseController::setTransform(const Transform& transform)
{
}

bool AnimationGraphPoseController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms)
{
	return m_stateGraph ? m_stateGraph->evaluate(time, deltaTime, worldTransform, skeleton, jointTransforms, outPoseTransforms) : false;
}

Vector4 AnimationGraphPoseController::consumeRootMotion()
{
	return m_stateGraph ? m_stateGraph->consumeRootMotion() : Vector4::zero();
}

IPoseController* AnimationGraphPoseController::getActivePoseController()
{
	return m_stateGraph ? m_stateGraph->getActivePoseController() : this;
}

void AnimationGraphPoseController::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers)
{
	if (is_type_of(type, type_of< AnimationGraphPoseController >()))
		outControllers.push_back(this);
	else if (m_stateGraph)
		m_stateGraph->getPoseControllersOf(type, outControllers);
}

}

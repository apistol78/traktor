/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Stack/StackAnimationController.h"

#include "Animation/Stack/RtStack.h"
#include "Core/Misc/SafeDestroy.h"

#include <limits>

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StackAnimationController", StackAnimationController, IPoseController)

StackAnimationController::StackAnimationController(RtStack* stack)
	: m_stack(stack)
{
}

void StackAnimationController::destroy()
{
	safeDestroy(m_stack);
}

void StackAnimationController::setOwner(world::Entity* owner)
{
	m_stack->setOwner(owner);
}

void StackAnimationController::setTransform(const Transform& transform)
{
	m_stack->setTransform(transform);
}

bool StackAnimationController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms)
{
	return m_stack ? m_stack->evaluate(time, deltaTime, worldTransform, skeleton, jointTransforms, outPoseTransforms) : false;
}

Vector4 StackAnimationController::consumeRootMotion()
{
	return m_stack ? m_stack->consumeRootMotion() : Vector4::zero();
}

IPoseController* StackAnimationController::getActivePoseController()
{
	return m_stack ? m_stack->getActivePoseController() : this;
}

void StackAnimationController::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers)
{
	if (is_type_of(type, type_of< StackAnimationController >()))
		outControllers.push_back(this);
	else if (m_stack)
		m_stack->getPoseControllersOf(type, outControllers);
}

void StackAnimationController::setActive(render::Handle name, bool active)
{
	if (m_stack)
		m_stack->setActive(name, active);
}

bool StackAnimationController::isActive(render::Handle name) const
{
	return m_stack ? m_stack->isActive(name) : false;
}

void StackAnimationController::activateOnly(render::Handle name)
{
	if (m_stack)
		m_stack->activateOnly(name);
}

}

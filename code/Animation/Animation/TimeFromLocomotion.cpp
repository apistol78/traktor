/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/TimeFromLocomotion.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.TimeFromLocomotion", TimeFromLocomotion, ITransformTime)

void TimeFromLocomotion::calculateTime(const Animation* animation, const Transform& worldTransform, float& inoutTime, float& outDeltaTime)
{
	const float distance = ((worldTransform.translation() - m_transform.translation()) * Vector4(1.0f, 0.0f, 1.0f)).length();

	outDeltaTime = animation->getTimePerDistance() * distance;
    inoutTime += outDeltaTime;

	m_transform = worldTransform;
}

}
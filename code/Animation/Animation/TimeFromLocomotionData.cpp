/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Animation/TimeFromLocomotion.h"
#include "Animation/Animation/TimeFromLocomotionData.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.TimeFromLocomotionData", 0, TimeFromLocomotionData, ITransformTimeData)

Ref< ITransformTime > TimeFromLocomotionData::createInstance() const
{
    return new TimeFromLocomotion();
}

void TimeFromLocomotionData::serialize(ISerializer& s)
{
}

}

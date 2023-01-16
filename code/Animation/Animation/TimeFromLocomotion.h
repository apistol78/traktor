/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Animation/Animation/ITransformTime.h"
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

/*!
 * \ingroup Animation
 */
class T_DLLCLASS TimeFromLocomotion : public ITransformTime
{
    T_RTTI_CLASS;

public:
    virtual void calculateTime(const Animation* animation, const Transform& worldTransform, float& inoutTime, float& outDeltaTime) override final;

private:
    Transform m_transform = Transform::identity();
    float m_time = 0.0f;
};

}

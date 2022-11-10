/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathConfig.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

#if !defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Transform.inl"
#endif

namespace traktor
{

void Transform::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"translation", m_translation, AttributeDirection());
	s >> Member< Quaternion >(L"rotation", m_rotation);
}

}

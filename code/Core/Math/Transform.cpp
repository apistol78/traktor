/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathConfig.h"
#include "Core/Math/Transform.h"

#if !(defined(_PS3) && defined(SPU))
#	include "Core/Serialization/AttributeDirection.h"
#	include "Core/Serialization/ISerializer.h"
#	include "Core/Serialization/Member.h"
#endif

#if !defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Transform.inl"
#endif

namespace traktor
{

#if !(defined(_PS3) && defined(SPU))

void Transform::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"translation", m_translation, AttributeDirection());
	s >> Member< Quaternion >(L"rotation", m_rotation);
}

#endif

}

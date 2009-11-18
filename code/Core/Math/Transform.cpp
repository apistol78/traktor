#include "Core/Math/MathConfig.h"
#include "Core/Math/Transform.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

#if !defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Transform.inl"
#endif

namespace traktor
{

bool Transform::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"translation", m_translation);
	s >> Member< Quaternion >(L"rotation", m_rotation);
	return true;
}

}

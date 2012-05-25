#include "Core/Math/MathConfig.h"
#include "Core/Math/Color4f.h"

#if !(defined(_PS3) && defined(SPU))
#	include "Core/Serialization/ISerializer.h"
#	include "Core/Serialization/Member.h"
#endif

#if !defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/Color4f.inl"
#endif

namespace traktor
{

#if !(defined(_PS3) && defined(SPU))

bool Color4f::serialize(ISerializer& s)
{
	return s >> Member< Vector4 >(L"data", m_data);
}

#endif

}

#include "Animation/Editor/AnimationAsset.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimationAsset", 1, AnimationAsset, editor::Asset)

AnimationAsset::AnimationAsset()
:	m_offset(Vector4::origo())
,	m_invertX(false)
,	m_invertZ(false)
{
}

void AnimationAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 1)
	{
		s >> Member< Vector4 >(L"offset", m_offset, AttributePoint());
		s >> Member< bool >(L"invertX", m_invertX);
		s >> Member< bool >(L"invertZ", m_invertZ);
	}
}

	}
}

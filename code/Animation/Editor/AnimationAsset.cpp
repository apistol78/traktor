#include "Animation/Editor/AnimationAsset.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimationAsset", 3, AnimationAsset, editor::Asset)

AnimationAsset::AnimationAsset()
:	m_animation(L"Animation")
,	m_scale(1.0f)
{
}

void AnimationAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 3)
	{
		s >> Member< std::wstring >(L"animation", m_animation);
		s >> Member< float >(L"scale", m_scale);
	}
	else
	{
		Vector4 offset;
		bool invertX;
		bool invertZ;
		bool autoCenterKeyPoses;

		if (s.getVersion() >= 1)
		{
			s >> Member< Vector4 >(L"offset", offset);
			s >> Member< bool >(L"invertX", invertX);
			s >> Member< bool >(L"invertZ", invertZ);
		}

		if (s.getVersion() >= 2)
			s >> Member< bool >(L"autoCenterKeyPoses", autoCenterKeyPoses);
	}
}

	}
}

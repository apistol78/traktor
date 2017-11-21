/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/AnimationAsset.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimationAsset", 2, AnimationAsset, editor::Asset)

AnimationAsset::AnimationAsset()
:	m_offset(Vector4::origo())
,	m_invertX(false)
,	m_invertZ(false)
,	m_autoCenterKeyPoses(true)
{
}

void AnimationAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion< AnimationAsset >() >= 1)
	{
		s >> Member< Vector4 >(L"offset", m_offset, AttributePoint());
		s >> Member< bool >(L"invertX", m_invertX);
		s >> Member< bool >(L"invertZ", m_invertZ);
	}

	if (s.getVersion< AnimationAsset >() >= 2)
		s >> Member< bool >(L"autoCenterKeyPoses", m_autoCenterKeyPoses);
}

	}
}

/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/AnimationAsset.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.AnimationAsset", 8, AnimationAsset, editor::Asset)

void AnimationAsset::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 3);

	editor::Asset::serialize(s);

	if (s.getVersion() >= 7)
		s >> Member< Guid >(L"targetSkeleton", m_targetSkeleton, AttributeType(type_of< SkeletonAsset >()));
	else if (s.getVersion() >= 4)
		s >> Member< Guid >(L"skeleton", m_targetSkeleton, AttributeType(type_of< SkeletonAsset >()));

	s >> Member< std::wstring >(L"take", m_take);

	if (s.getVersion() >= 8)
		s >> Member< Vector4 >(L"scale", m_scale, AttributePoint());
	else
	{
		float scaleFactor;
		s >> Member< float >(L"scale", scaleFactor);
		m_scale = Vector4(scaleFactor, scaleFactor, scaleFactor, 1.0f);
	}

	if (s.getVersion() >= 5)
		s >> Member< Vector4 >(L"translate", m_translate);

	if (s.getVersion() >= 6)
		s >> Member< bool >(L"removeLocomotion", m_removeLocomotion);
}

}

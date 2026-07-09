/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/SkeletonAsset.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.SkeletonAsset", 5, SkeletonAsset, editor::Asset)

void SkeletonAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 1)
	{
		s >> Member< Vector4 >(L"offset", m_offset, AttributePoint());

		if (s.getVersion() >= 4)
			s >> Member< Vector4 >(L"scale", m_scale, AttributePoint());
		else if (s.getVersion() >= 3)
		{
			float scaleFactor;
			s >> Member< float >(L"scale", scaleFactor);
			m_scale = Vector4(scaleFactor, scaleFactor, scaleFactor, 1.0f);
		}

		if (s.getVersion() >= 2)
		{
			if (s.getVersion() < 5)
				s >> ObsoleteMember< float >(L"radius");
		}
		else
			s >> ObsoleteMember< float >(L"boneRadius");

		s >> Member< bool >(L"invertX", m_invertX);
		s >> Member< bool >(L"invertZ", m_invertZ);
	}
}


}

/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Editor/MeshAsset.h"

#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Physics/Editor/Material.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.physics.MeshAsset", 9, MeshAsset, editor::Asset)

void MeshAsset::serialize(ISerializer& s)
{
	const MemberEnum< CenterMode >::Key c_CenterMode_Keys[] = {
		{ L"None", CenterMode::None },
		{ L"XZ", CenterMode::XZ },
		{ L"XYZ", CenterMode::XYZ },
		{ 0 }
	};

	editor::Asset::serialize(s);

	if (s.getVersion< MeshAsset >() >= 4)
		s >> Member< std::wstring >(L"importFilter", m_importFilter);

	if (s.getVersion< MeshAsset >() >= 1)
		s >> Member< bool >(L"calculateConvexHull", m_calculateConvexHull);

	if (s.getVersion< MeshAsset >() >= 2)
		s >> Member< float >(L"margin", m_margin);

	if (s.getVersion< MeshAsset >() >= 7)
		s >> Member< Vector4 >(L"scaleFactor", m_scaleFactor, AttributePoint());
	else if (s.getVersion< MeshAsset >() >= 5)
	{
		float scaleFactor;
		s >> Member< float >(L"scaleFactor", scaleFactor);
		m_scaleFactor = Vector4(scaleFactor, scaleFactor, scaleFactor, 1.0f);
	}

	if (s.getVersion< MeshAsset >() >= 6)
		s >> Member< float >(L"reduce", m_reduce, AttributeRange(0.0f, 1.0f));

	if (s.getVersion< MeshAsset >() >= 8)
	{
		if (s.getVersion() >= 9)
			s >> MemberEnum< CenterMode >(L"center", m_center, c_CenterMode_Keys);
		else
		{
			bool center;
			s >> Member< bool >(L"center", center);
			m_center = center ? CenterMode::XYZ : CenterMode::None;
		}
	}

	if (s.getVersion< MeshAsset >() >= 5)
		s >> Member< bool >(L"grounded", m_grounded);

	if (s.getVersion< MeshAsset >() >= 3)
		s >> MemberSmallMap< std::wstring, Guid >(L"materials", m_materials, AttributeType(type_of< Material >()));
}

}

/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Editor/Texture/ColorGradingTextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ColorGradingTextureAsset", 3, ColorGradingTextureAsset, ISerializable)

ColorGradingTextureAsset::ColorGradingTextureAsset()
{
	m_redCurve.  push_back({ 0.0f, 0.0f }); m_redCurve.  push_back({ 1.0f, 1.0f });
	m_greenCurve.push_back({ 0.0f, 0.0f }); m_greenCurve.push_back({ 1.0f, 1.0f });
	m_blueCurve. push_back({ 0.0f, 0.0f }); m_blueCurve. push_back({ 1.0f, 1.0f });
}

void ColorGradingTextureAsset::serialize(ISerializer& s)
{
	T_FATAL_ASSERT (s.getVersion< ColorGradingTextureAsset >() >= 3);

	s >> MemberAlignedVector< std::pair< float, float >, MemberStlPair< float, float > >(L"redCurve", m_redCurve);
	s >> MemberAlignedVector< std::pair< float, float >, MemberStlPair< float, float > >(L"greenCurve", m_greenCurve);
	s >> MemberAlignedVector< std::pair< float, float >, MemberStlPair< float, float > >(L"blueCurve", m_blueCurve);

	s >> Member< float >(L"brightness", m_brightness);
	s >> Member< float >(L"contrast", m_contrast);
	s >> Member< float >(L"saturation", m_saturation);
}

}

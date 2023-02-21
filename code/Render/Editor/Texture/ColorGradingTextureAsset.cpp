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
#include "Render/Editor/Texture/ColorGradingTextureAsset.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ColorGradingTextureAsset", 1, ColorGradingTextureAsset, ISerializable)

void ColorGradingTextureAsset::serialize(ISerializer& s)
{
	if (s.getVersion< ColorGradingTextureAsset >() >= 1)
	{
		s >> Member< float >(L"redGamma", m_redGamma, AttributeRange(0.0f));
		s >> Member< float >(L"greenGamma", m_greenGamma, AttributeRange(0.0f));
		s >> Member< float >(L"blueGamma", m_blueGamma, AttributeRange(0.0f));
	}
	else
	{
		float gamma;
		s >> Member< float >(L"gamma", gamma, AttributeRange(0.0f));
		m_redGamma = gamma;
		m_greenGamma = gamma;
		m_blueGamma = gamma;
	}
}

}

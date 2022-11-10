/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader.h"
#include "Render/Editor/Image2/ImgStepDirectionalBlur.h"
#include "Resource/Member.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImgStepDirectionalBlur", 0, ImgStepDirectionalBlur, IImgStep)

ImgStepDirectionalBlur::ImgStepDirectionalBlur()
:   m_blurType(BtGaussian)
,   m_direction(1.0f, 0.0f)
,   m_taps(15)
{
}

std::wstring ImgStepDirectionalBlur::getTitle() const
{
    return L"Directional Blur";
}

void ImgStepDirectionalBlur::getInputs(std::set< std::wstring >& outInputs) const
{
    for (const auto& parameter : m_parameters)
        outInputs.insert(parameter);
}

void ImgStepDirectionalBlur::serialize(ISerializer& s)
{
	const MemberEnum< BlurType >::Key c_BlurType_Keys[] =
	{
		{ L"BtGaussian", BtGaussian },
		{ L"BtSine", BtSine },
		{ L"BtBox", BtBox },
		{ L"BtBox2D", BtBox2D },
		{ L"BtCircle2D", BtCircle2D },
		{ 0 }
	};

	s >> MemberEnum< BlurType >(L"blurType", m_blurType, c_BlurType_Keys);
	s >> Member< Vector2 >(L"direction", m_direction);
	s >> Member< int32_t >(L"taps", m_taps);
    s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlList< std::wstring >(L"parameters", m_parameters);
}

    }
}

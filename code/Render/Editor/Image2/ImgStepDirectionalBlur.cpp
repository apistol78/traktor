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

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImgStepDirectionalBlur", 1, ImgStepDirectionalBlur, IImgStep)

ImgStepDirectionalBlur::ImgStepDirectionalBlur()
:   m_blurType(BlurType::Gaussian)
,   m_direction(1.0f, 0.0f)
,   m_taps(15)
{
}

void ImgStepDirectionalBlur::getInputs(std::set< std::wstring >& outInputs) const
{
    for (const auto& parameter : m_parameters)
        outInputs.insert(parameter);
}

void ImgStepDirectionalBlur::serialize(ISerializer& s)
{
	IImgStep::serialize(s);

	if (s.getVersion< ImgStepDirectionalBlur >() >= 1)
	{
		const MemberEnum< BlurType >::Key c_BlurType_Keys[] =
		{
			{ L"Gaussian", BlurType::Gaussian },
			{ L"Sine", BlurType::Sine },
			{ L"Box", BlurType::Box },
			{ L"Box2D", BlurType::Box2D },
			{ L"Circle2D", BlurType::Circle2D },
			{ 0 }
		};
		s >> MemberEnum< BlurType >(L"blurType", m_blurType, c_BlurType_Keys);
	}
	else
	{
		const MemberEnum< BlurType >::Key c_BlurType_Keys[] =
		{
			{ L"BtGaussian", BlurType::Gaussian },
			{ L"BtSine", BlurType::Sine },
			{ L"BtBox", BlurType::Box },
			{ L"BtBox2D", BlurType::Box2D },
			{ L"BtCircle2D", BlurType::Circle2D },
			{ 0 }
		};
		s >> MemberEnum< BlurType >(L"blurType", m_blurType, c_BlurType_Keys);
	}

	s >> Member< Vector2 >(L"direction", m_direction);
	s >> Member< int32_t >(L"taps", m_taps);
    s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlList< std::wstring >(L"parameters", m_parameters);
}

}

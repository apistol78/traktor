/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Shader.h"
#include "Render/Editor/Image2/ImgStepCompute.h"
#include "Resource/Member.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ImgStepCompute", 1, ImgStepCompute, IImgStep)

void ImgStepCompute::getInputs(std::set< std::wstring >& outInputs) const
{
	if (m_workSize == WorkSize::SizeOf)
		outInputs.insert(L"WorkSize");

	for (const auto& parameter : m_parameters)
		outInputs.insert(parameter);
}

void ImgStepCompute::serialize(ISerializer& s)
{
	IImgStep::serialize(s);

	s >> resource::Member< render::Shader >(L"shader", m_shader);

	if (s.getVersion< ImgStepCompute >() >= 1)
	{
		const MemberEnum< WorkSize >::Key kWorkSize_Keys[] =
		{
			{ L"Manual", WorkSize::Manual },
			{ L"Output", WorkSize::Output },
			{ L"SizeOf", WorkSize::SizeOf },
			{ 0 }
		};
		s >> MemberEnum< WorkSize >(L"workSize", m_workSize, kWorkSize_Keys);      

		const wchar_t* c_localSizeElements[] = { L"X", L"Y", L"Z" };
		s >> MemberStaticArray< int32_t, 3 >(L"manualWorkSize", m_manualWorkSize, c_localSizeElements);
	}

	s >> MemberStlList< std::wstring >(L"parameters", m_parameters);
}

}

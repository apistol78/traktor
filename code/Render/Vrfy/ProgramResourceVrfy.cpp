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
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Render/Vrfy/ProgramResourceVrfy.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ProgramResourceVrfy", 0, ProgramResourceVrfy, ProgramResource)

void ProgramResourceVrfy::serialize(ISerializer& s)
{
	ProgramResource::serialize(s);

	s >> MemberRef< ProgramResource >(L"embedded", m_embedded);
	s >> Member< std::wstring >(L"vertexShader", m_vertexShader);
	s >> Member< std::wstring >(L"pixelShader", m_pixelShader);
	s >> Member< std::wstring >(L"computeShader", m_computeShader);
	s >> MemberAlignedVector< Uniform, MemberComposite< Uniform > >(L"uniforms", m_uniforms);
}

void ProgramResourceVrfy::Uniform::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> MemberEnumByValue< ParameterType >(L"type", type);
	s >> Member< int32_t >(L"length", length);
}

}

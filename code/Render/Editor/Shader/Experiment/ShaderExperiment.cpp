/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/Experiment/ShaderExperiment.h"

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/StructDeclaration.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ShaderExperiment", 0, ShaderExperiment, ISerializable)

void ShaderExperiment::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"shader", m_shader, AttributeType(type_of< ShaderGraph >()));
	s >> MemberAlignedVector< Data, MemberComposite< Data > >(L"data", m_data);
	s >> MemberAlignedVector< Pass, MemberComposite< Pass > >(L"pass", m_passes);
}

void ShaderExperiment::Data::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Guid >(L"structDeclaration", structDeclaration, AttributeType(type_of< StructDeclaration >()));
	s >> Member< int32_t >(L"count", count);
}

void ShaderExperiment::Pass::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"technique", technique);
	s >> Member< int32_t >(L"workSize", workSize);
}

}

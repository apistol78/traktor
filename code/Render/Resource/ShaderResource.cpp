/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Render/Resource/ShaderResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderResource", 3, ShaderResource, ISerializable)

const SmallMap< std::wstring, uint32_t >& ShaderResource::getParameterBits() const
{
	return m_parameterBits;
}

const AlignedVector< ShaderResource::Technique >& ShaderResource::getTechniques() const
{
	return m_techniques;
}

void ShaderResource::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 3);
	s >> MemberSmallMap< std::wstring, uint32_t >(L"parameterBits", m_parameterBits);
	s >> MemberAlignedVector< Technique, MemberComposite< Technique > >(L"techniques", m_techniques);
}

void ShaderResource::InitializeUniformScalar::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< float >(L"value", value);
}

void ShaderResource::InitializeUniformVector::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< Vector4 >(L"value", value);
}

void ShaderResource::Combination::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"mask", mask);
	s >> Member< uint32_t >(L"value", value);
	s >> Member< uint32_t >(L"priority", priority);
	s >> MemberRef< ISerializable >(L"program", program);
	s >> MemberAlignedVector< Guid >(L"textures", textures);
	s >> MemberAlignedVector< InitializeUniformScalar, MemberComposite< InitializeUniformScalar > >(L"initializeUniformScalar", initializeUniformScalar);
	s >> MemberAlignedVector< InitializeUniformVector, MemberComposite< InitializeUniformVector > >(L"initializeUniformVector", initializeUniformVector);
}

void ShaderResource::Technique::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"mask", mask);
	s >> MemberAlignedVector< Combination, MemberComposite< Combination > >(L"combinations", combinations);
}

	}
}

/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Resource/ShaderResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderResource", 3, ShaderResource, ISerializable)

const std::map< std::wstring, uint32_t >& ShaderResource::getParameterBits() const
{
	return m_parameterBits;
}

const std::vector< ShaderResource::Technique >& ShaderResource::getTechniques() const
{
	return m_techniques;
}

void ShaderResource::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 3);
	s >> MemberStlMap< std::wstring, uint32_t >(L"parameterBits", m_parameterBits);
	s >> MemberStlVector< Technique, MemberComposite< Technique > >(L"techniques", m_techniques);
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
	s >> MemberStlVector< Guid >(L"textures", textures);
	s >> MemberAlignedVector< InitializeUniformScalar, MemberComposite< InitializeUniformScalar > >(L"initializeUniformScalar", initializeUniformScalar);
	s >> MemberAlignedVector< InitializeUniformVector, MemberComposite< InitializeUniformVector > >(L"initializeUniformVector", initializeUniformVector);
}

void ShaderResource::Technique::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"mask", mask);
	s >> MemberStlVector< Combination, MemberComposite< Combination > >(L"combinations", combinations);
}

	}
}

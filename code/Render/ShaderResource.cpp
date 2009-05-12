#include "Render/ShaderResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.ShaderResource", ShaderResource, Serializable)

const std::map< std::wstring, uint32_t >& ShaderResource::getParameterBits() const
{
	return m_parameterBits;
}

const std::vector< ShaderResource::Technique >& ShaderResource::getTechniques() const
{
	return m_techniques;
}

bool ShaderResource::serialize(Serializer& s)
{
	s >> MemberStlMap< std::wstring, uint32_t >(L"parameterBits", m_parameterBits);
	s >> MemberStlVector< Technique, MemberComposite< Technique > >(L"techniques", m_techniques);
	return true;
}

bool ShaderResource::Combination::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"parameterValue", parameterValue);
	s >> MemberRef< Serializable >(L"program", program);
	return true;
}

bool ShaderResource::Technique::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"parameterMask", parameterMask);
	s >> MemberStlVector< Combination, MemberComposite< Combination > >(L"combinations", combinations);
	return true;
}

	}
}

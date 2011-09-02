#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Render/Resource/ShaderResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderResource", 0, ShaderResource, ISerializable)

const std::map< std::wstring, uint32_t >& ShaderResource::getParameterBits() const
{
	return m_parameterBits;
}

const std::vector< ShaderResource::Technique >& ShaderResource::getTechniques() const
{
	return m_techniques;
}

bool ShaderResource::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, uint32_t >(L"parameterBits", m_parameterBits);
	s >> MemberStlVector< Technique, MemberComposite< Technique > >(L"techniques", m_techniques);
	return true;
}

bool ShaderResource::Combination::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"parameterValue", parameterValue);
	s >> MemberRef< ISerializable >(L"program", program);
	s >> MemberStlVector< Guid >(L"textures", textures);
	return true;
}

bool ShaderResource::Technique::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< uint32_t >(L"parameterMask", parameterMask);
	s >> MemberStlVector< Combination, MemberComposite< Combination > >(L"combinations", combinations);
	return true;
}

	}
}

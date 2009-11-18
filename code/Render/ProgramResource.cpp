#include "Render/ProgramResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramResource", ProgramResource, ISerializable)

void ProgramResource::addTexture(const std::wstring& parameterName, const Guid& guid)
{
	m_textures.push_back(std::make_pair(parameterName, guid));
}

const std::vector< std::pair< std::wstring, Guid > >& ProgramResource::getTextures() const
{
	return m_textures;
}

bool ProgramResource::serialize(ISerializer& s)
{
	return s >> MemberStlVector< std::pair< std::wstring, Guid >, MemberStlPair< std::wstring, Guid > >(L"textures", m_textures);
}

	}
}

#include "Render/ProgramResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramResource", ProgramResource, Serializable)

const std::vector< std::pair< std::wstring, Guid > >& ProgramResource::getTextures() const
{
	return m_textures;
}

bool ProgramResource::serialize(Serializer& s)
{
	return s >> MemberStlVector< std::pair< std::wstring, Guid >, MemberStlPair< std::wstring, Guid > >(L"textures", m_textures);
}

	}
}

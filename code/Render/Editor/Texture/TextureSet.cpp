#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Editor/Texture/TextureSet.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TextureSet", 0, TextureSet, ISerializable)

void TextureSet::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, Guid >(L"textures", m_textures);
}

	}
}

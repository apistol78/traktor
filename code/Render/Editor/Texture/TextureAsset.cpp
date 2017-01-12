#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Editor/Texture/TextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TextureAsset", 13, TextureAsset, editor::Asset)

void TextureAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);
	s >> MemberComposite< TextureOutput, false >(L"output", m_output);
}

	}
}

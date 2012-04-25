#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/Resource/TextureResource.h"
#include "Render/Editor/Texture/TextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TextureAsset", 5, TextureAsset, editor::Asset)

const TypeInfo* TextureAsset::getOutputType() const
{
	return &type_of< TextureResource >();
}

bool TextureAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> MemberComposite< TextureOutput, false >(L"output", m_output);
	return true;
}

	}
}

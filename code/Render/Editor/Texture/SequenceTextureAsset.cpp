#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Editor/Texture/SequenceTextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.SequenceTextureAsset", 13, SequenceTextureAsset, ISerializable)

SequenceTextureAsset::SequenceTextureAsset()
:	m_rate(10.0f)
{
}

void SequenceTextureAsset::serialize(ISerializer& s)
{
	s >> MemberStlList< Path, Member< Path > >(L"fileNames", m_fileNames);
	s >> MemberComposite< TextureOutput, false >(L"output", m_output);
	s >> Member< float >(L"rate", m_rate);
}

	}
}

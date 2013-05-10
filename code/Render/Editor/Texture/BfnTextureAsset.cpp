#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/BfnTextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.BfnTextureAsset", 0, BfnTextureAsset, ISerializable)

BfnTextureAsset::BfnTextureAsset()
:	m_bestFitFactorOnly(true)
{
}

void BfnTextureAsset::serialize(ISerializer& s)
{
	s >> Member< bool >(L"bestFitFactorOnly", m_bestFitFactorOnly);
}

	}
}

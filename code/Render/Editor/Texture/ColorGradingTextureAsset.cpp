#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/ColorGradingTextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ColorGradingTextureAsset", 0, ColorGradingTextureAsset, ISerializable)

ColorGradingTextureAsset::ColorGradingTextureAsset()
:	m_gamma(1.0f)
{
}

void ColorGradingTextureAsset::serialize(ISerializer& s)
{
	s >> Member< float >(L"gamma", m_gamma);
}

	}
}

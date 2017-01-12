#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/ProbeTextureAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.ProbeTextureAsset", 13, ProbeTextureAsset, editor::Asset)

ProbeTextureAsset::ProbeTextureAsset()
:	m_filterAngle(20.0f)
,	m_filterMipDeltaAngle(1.0f)
{
}

void ProbeTextureAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< float >(L"filterAngle", m_filterAngle, AttributeRange(0.0f));
	s >> Member< float >(L"filterMipDeltaAngle", m_filterMipDeltaAngle, AttributeRange(0.0f));
}

	}
}

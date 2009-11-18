#include "Render/Editor/TextureAsset.h"
#include "Render/TextureResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.TextureAsset", 1, TextureAsset, editor::Asset)

TextureAsset::TextureAsset()
:	m_generateNormalMap(false)
,	m_scaleDepth(0.0f)
,	m_generateMips(true)
,	m_keepZeroAlpha(true)
,	m_isCubeMap(false)
,	m_hasAlpha(false)
,	m_ignoreAlpha(false)
,	m_scaleImage(false)
,	m_scaleWidth(0)
,	m_scaleHeight(0)
,	m_enableCompression(true)
,	m_linearGamma(true)
,	m_generateSphereMap(false)
{
}

const TypeInfo* TextureAsset::getOutputType() const
{
	return &type_of< TextureResource >();
}

bool TextureAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> Member< bool >(L"generateNormalMap", m_generateNormalMap);
	s >> Member< float >(L"scaleDepth", m_scaleDepth);
	s >> Member< bool >(L"generateMips", m_generateMips);
	s >> Member< bool >(L"keepZeroAlpha", m_keepZeroAlpha);
	s >> Member< bool >(L"isCubeMap", m_isCubeMap);
	s >> Member< bool >(L"hasAlpha", m_hasAlpha);
	s >> Member< bool >(L"ignoreAlpha", m_ignoreAlpha);
	s >> Member< bool >(L"scaleImage", m_scaleImage);
	s >> Member< int32_t >(L"scaleWidth", m_scaleWidth);
	s >> Member< int32_t >(L"scaleHeight", m_scaleHeight);
	s >> Member< bool >(L"enableCompression", m_enableCompression);
	s >> Member< bool >(L"linearGamma", m_linearGamma);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"generateSphereMap", m_generateSphereMap);
	}

	return true;
}

	}
}

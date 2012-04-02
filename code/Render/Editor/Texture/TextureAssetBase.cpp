#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/TextureAssetBase.h"
#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TextureAssetBase", TextureAssetBase, editor::Asset)

TextureAssetBase::TextureAssetBase()
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
,	m_enableNormalMapCompression(false)
,	m_inverseNormalMapY(false)
,	m_linearGamma(true)
,	m_generateSphereMap(false)
,	m_preserveAlphaCoverage(false)
,	m_alphaCoverageReference(0.5f)
{
}

const TypeInfo* TextureAssetBase::getOutputType() const
{
	return &type_of< TextureResource >();
}

bool TextureAssetBase::serialize(ISerializer& s)
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

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"enableNormalMapCompression", m_enableNormalMapCompression);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"inverseNormalMapY", m_inverseNormalMapY);

	s >> Member< bool >(L"linearGamma", m_linearGamma);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"generateSphereMap", m_generateSphereMap);

	if (s.getVersion() >= 5)
	{
		s >> Member< bool >(L"preserveAlphaCoverage", m_preserveAlphaCoverage);
		s >> Member< float >(L"alphaCoverageReference", m_alphaCoverageReference);
	}

	return true;
}

	}
}

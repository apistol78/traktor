#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/RadianceProbeAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.RadianceProbeAsset", 1, RadianceProbeAsset, editor::Asset)

RadianceProbeAsset::RadianceProbeAsset()
:	m_glossScale(20)
,	m_glossBias(1)
{
}

void RadianceProbeAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< int32_t >(L"glossScale", m_glossScale);
	s >> Member< int32_t >(L"glossBias", m_glossBias);

	if (s.getVersion< RadianceProbeAsset >() < 1)
	{
		int32_t sizeDivisor;
		s >> Member< int32_t >(L"sizeDivisor", sizeDivisor);
	}
}

	}
}

#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/RadianceProbeAsset.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.render.RadianceProbeAsset", 0, RadianceProbeAsset, editor::Asset)

RadianceProbeAsset::RadianceProbeAsset()
:	m_solidAngle(0.0f)
{
}

void RadianceProbeAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);
	s >> Member< float >(L"solidAngle", m_solidAngle, AttributeRange(0.0f));
}

	}
}

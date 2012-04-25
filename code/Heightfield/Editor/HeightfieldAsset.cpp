#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Editor/HeightfieldAsset.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldAsset", 2, HeightfieldAsset, editor::Asset)

HeightfieldAsset::HeightfieldAsset()
:	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
,	m_invertX(false)
,	m_invertZ(false)
{
}

bool HeightfieldAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> Member< Vector4 >(L"worldExtent", m_worldExtent, AttributeDirection());
	
	if (s.getVersion() < 2)
	{
		uint32_t patchDim = 0;
		s >> Member< uint32_t >(L"patchDim", patchDim);
	}

	s >> Member< uint32_t >(L"detailSkip", m_detailSkip);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"invertX", m_invertX);
		s >> Member< bool >(L"invertZ", m_invertZ);
	}

	return true;
}

	}
}

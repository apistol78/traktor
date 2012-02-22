#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/HeightfieldResource.h"
#include "Heightfield/Editor/HeightfieldAsset.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldAsset", 1, HeightfieldAsset, editor::Asset)

HeightfieldAsset::HeightfieldAsset()
:	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
,	m_patchDim(129)
,	m_invertX(false)
,	m_invertZ(false)
{
}

const TypeInfo* HeightfieldAsset::getOutputType() const
{
	return &type_of< HeightfieldResource >();
}

bool HeightfieldAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> Member< Vector4 >(L"worldExtent", m_worldExtent, AttributeDirection());
	s >> Member< uint32_t >(L"patchDim", m_patchDim);
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

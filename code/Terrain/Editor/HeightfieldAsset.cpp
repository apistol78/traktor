#include "Terrain/Editor/HeightfieldAsset.h"
#include "Terrain/HeightfieldResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.terrain.HeightfieldAsset", HeightfieldAsset, editor::Asset)

HeightfieldAsset::HeightfieldAsset()
:	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
,	m_patchDim(129)
{
}

const Type* HeightfieldAsset::getOutputType() const
{
	return &type_of< HeightfieldResource >();
}

bool HeightfieldAsset::serialize(Serializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> Member< Vector4 >(L"worldExtent", m_worldExtent);
	s >> Member< uint32_t >(L"patchDim", m_patchDim);
	s >> Member< uint32_t >(L"detailSkip", m_detailSkip);

	return true;
}

	}
}

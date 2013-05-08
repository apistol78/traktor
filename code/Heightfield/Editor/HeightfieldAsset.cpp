#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Heightfield/Editor/HeightfieldAsset.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.hf.HeightfieldAsset", 4, HeightfieldAsset, ISerializable)

HeightfieldAsset::HeightfieldAsset()
:	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
{
}

HeightfieldAsset::HeightfieldAsset(const Vector4& worldExtent)
:	m_worldExtent(worldExtent)
{
}

void HeightfieldAsset::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 4);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent, AttributeDirection());
}

	}
}

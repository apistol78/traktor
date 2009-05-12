#include "Physics/Editor/HeightfieldAsset.h"
#include "Physics/HeightfieldResource.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.HeightfieldAsset", HeightfieldAsset, editor::Asset)

HeightfieldAsset::HeightfieldAsset()
:	m_worldExtent(0.0f, 0.0f, 0.0f, 0.0f)
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

	return true;
}

	}
}

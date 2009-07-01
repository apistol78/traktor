#include "World/Entity/SpatialEntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDITABLE_CLASS(L"traktor.world.SpatialEntityData", SpatialEntityData, EntityData)

SpatialEntityData::SpatialEntityData()
:	m_transform(Matrix44::identity())
{
}

void SpatialEntityData::setTransform(const Matrix44& transform)
{
	m_transform = transform;
}
	
const Matrix44& SpatialEntityData::getTransform() const
{
	return m_transform;
}

bool SpatialEntityData::serialize(Serializer& s)
{
	return s >> Member< Matrix44 >(L"transform", m_transform);
}

	}
}

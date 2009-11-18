#include "World/Entity/SpatialEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.SpatialEntityData", 0, SpatialEntityData, EntityData)

SpatialEntityData::SpatialEntityData()
:	m_transform(Transform::identity())
{
}

void SpatialEntityData::setTransform(const Transform& transform)
{
	m_transform = transform;
}
	
const Transform& SpatialEntityData::getTransform() const
{
	return m_transform;
}

bool SpatialEntityData::serialize(ISerializer& s)
{
	return s >> MemberComposite< Transform >(L"transform", m_transform);
}

	}
}

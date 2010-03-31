#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "World/Entity/SpatialEntityData.h"

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
	if (!EntityData::serialize(s))
		return false;

	return s >> MemberComposite< Transform >(L"transform", m_transform);
}

	}
}

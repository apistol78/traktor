#include "World/Entity/VolumeEntity.h"
#include "World/Entity/VolumeEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VolumeEntity", VolumeEntity, Entity)

VolumeEntity::VolumeEntity(const VolumeEntityData* data)
:	m_data(data)
{
	m_transform = m_data->getTransform();
	const AlignedVector< Aabb3 >& volumes = m_data->getVolumes();
	for (AlignedVector< Aabb3 >::const_iterator i = volumes.begin(); i != volumes.end(); ++i)
		m_boundingBox.contain(*i);
}

void VolumeEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool VolumeEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 VolumeEntity::getBoundingBox() const
{
	return m_boundingBox;
}

void VolumeEntity::update(const UpdateParams& update)
{
}

bool VolumeEntity::inside(const Vector4& point) const
{
	Vector4 p = m_transform.inverse() * point.xyz1();
	const AlignedVector< Aabb3 >& volumes = m_data->getVolumes();
	for (AlignedVector< Aabb3 >::const_iterator i = volumes.begin(); i != volumes.end(); ++i)
	{
		if (i->inside(p))
			return true;
	}
	return false;
}

	}
}

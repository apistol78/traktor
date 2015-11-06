#include "World/Entity.h"
#include "World/Entity/VolumeComponent.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VolumeComponent", VolumeComponent, IEntityComponent)

VolumeComponent::VolumeComponent(Entity* owner, const VolumeComponentData* data)
:	m_owner(owner)
,	m_data(data)
{
	const AlignedVector< Aabb3 >& volumes = m_data->getVolumes();
	for (AlignedVector< Aabb3 >::const_iterator i = volumes.begin(); i != volumes.end(); ++i)
		m_boundingBox.contain(*i);
}

void VolumeComponent::destroy()
{
}

void VolumeComponent::setTransform(const Transform& transform)
{
}

Aabb3 VolumeComponent::getBoundingBox() const
{
	return m_boundingBox;
}

void VolumeComponent::update(const UpdateParams& update)
{
}

bool VolumeComponent::inside(const Vector4& point) const
{
	Transform transform;
	if (!m_owner->getTransform(transform))
		return false;

	Vector4 p = transform.inverse() * point.xyz1();
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

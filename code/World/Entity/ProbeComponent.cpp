#include "World/Entity.h"
#include "World/Entity/ProbeComponent.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ProbeComponent", ProbeComponent, IEntityComponent)

ProbeComponent::ProbeComponent(
	const resource::Proxy< render::ITexture >& probeDiffuseTexture,
	const resource::Proxy< render::ITexture >& probeSpecularTexture,
	const Aabb3& volume
)
:	m_owner(nullptr)
,	m_probeDiffuseTexture(probeDiffuseTexture)
,	m_probeSpecularTexture(probeSpecularTexture)
,	m_volume(volume)
{
}

void ProbeComponent::destroy()
{
}

void ProbeComponent::setOwner(Entity* owner)
{
	m_owner = owner;
}

void ProbeComponent::update(const UpdateParams& update)
{
}

void ProbeComponent::setTransform(const Transform& transform)
{
}

Aabb3 ProbeComponent::getBoundingBox() const
{
	return m_volume;
}

Transform ProbeComponent::getTransform() const
{
	Transform transform;
	m_owner->getTransform(transform);
	return transform;
}

	}
}

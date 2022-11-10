#include "Mesh/MeshComponent.h"
#include "World/Entity.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponent", MeshComponent, world::IEntityComponent)

MeshComponent::MeshComponent(bool screenSpaceCulling)
:	m_owner(nullptr)
,	m_screenSpaceCulling(screenSpaceCulling)
,	m_transform(Transform::identity())
{
}

void MeshComponent::destroy()
{
	m_owner = nullptr;
}

void MeshComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
	{
		m_transform = IntervalTransform(
			m_owner->getTransform()
		);
	}
}

void MeshComponent::setTransform(const Transform& transform)
{
	m_transform.set(transform);
}

void MeshComponent::update(const world::UpdateParams& update)
{
	m_transform.step();
}

}

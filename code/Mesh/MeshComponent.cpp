#include "Mesh/MeshComponent.h"
#include "World/Entity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponent", MeshComponent, world::IEntityComponent)

MeshComponent::MeshComponent(world::Entity* owner, bool screenSpaceCulling)
:	m_owner(owner)
,	m_screenSpaceCulling(screenSpaceCulling)
{
	Transform transform;
	if (m_owner->getTransform(transform))
		m_transform = transform;
}

void MeshComponent::destroy()
{
	m_owner = 0;
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
}

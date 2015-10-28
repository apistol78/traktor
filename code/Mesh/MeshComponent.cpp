#include "Mesh/MeshComponent.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponent", MeshComponent, world::IEntityComponent)

MeshComponent::MeshComponent(world::Entity* owner, bool screenSpaceCulling)
:	m_owner(owner)
,	m_screenSpaceCulling(screenSpaceCulling)
{
}

void MeshComponent::destroy()
{
	m_owner = 0;
}

void MeshComponent::update(const world::UpdateParams& update)
{
}

	}
}

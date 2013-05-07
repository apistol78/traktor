#include "Ai/NavMeshEntity.h"

namespace traktor
{
	namespace ai
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshEntity", NavMeshEntity, world::Entity)

NavMeshEntity::NavMeshEntity(const resource::Proxy< NavMesh >& navMesh)
:	m_navMesh(navMesh)
{
}

Aabb3 NavMeshEntity::getBoundingBox() const
{
	return Aabb3();
}

void NavMeshEntity::update(const world::UpdateParams& update)
{
}

	}
}

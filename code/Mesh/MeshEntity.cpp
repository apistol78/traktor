#include "Mesh/MeshEntity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntity", MeshEntity, world::SpatialEntity)

MeshEntity::MeshEntity(const Transform& transform)
:	m_transform(transform)
,	m_transformPrevious(transform)
,	m_userParameter(0.0f)
,	m_parameterCallback(0)
{
}

void MeshEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool MeshEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

	}
}
